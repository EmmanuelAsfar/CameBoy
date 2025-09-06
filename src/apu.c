#include "apu.h"
#include <math.h>
#include <string.h>

// Patterns de duty cycle pour les canaux Square
static const u8 DUTY_PATTERNS[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1}, // 12.5%
    {1, 0, 0, 0, 0, 0, 0, 1}, // 25%
    {1, 0, 0, 0, 0, 1, 1, 1}, // 50%
    {0, 1, 1, 1, 1, 1, 1, 0}  // 75%
};

// Initialisation de l'APU
void apu_init(APU* apu) {
    memset(apu, 0, sizeof(APU));
    
    // Configuration par défaut
    apu->sample_rate = 44100;
    apu->buffer_size = 1024;
    apu->audio_buffer = malloc(apu->buffer_size * 2 * sizeof(s16)); // Stéréo
    
    // Initialisation des canaux
    memset(&apu->square1, 0, sizeof(SquareChannel));
    memset(&apu->square2, 0, sizeof(SquareChannel));
    memset(&apu->wave, 0, sizeof(WaveChannel));
    memset(&apu->noise, 0, sizeof(NoiseChannel));
    
    // Valeurs par défaut des registres
    apu->nr50 = 0x77; // Volume max, pas de vin
    apu->nr51 = 0xF3; // Tous les canaux activés
    apu->nr52 = 0xF1; // APU activé, tous les canaux activés
    
    apu->apu_enabled = true;
    apu->frame_sequencer = 0;
    apu->sample_counter = 0;
    apu->buffer_position = 0;
}

// Nettoyage de l'APU
void apu_cleanup(APU* apu) {
    if (apu->audio_buffer) {
        free(apu->audio_buffer);
        apu->audio_buffer = NULL;
    }
}

// Reset de l'APU
void apu_reset(APU* apu) {
    apu->nr50 = 0x00;
    apu->nr51 = 0x00;
    apu->nr52 = 0x00;
    
    memset(&apu->square1, 0, sizeof(SquareChannel));
    memset(&apu->square2, 0, sizeof(SquareChannel));
    memset(&apu->wave, 0, sizeof(WaveChannel));
    memset(&apu->noise, 0, sizeof(NoiseChannel));
    
    apu->apu_enabled = false;
    apu->frame_sequencer = 0;
    apu->sample_counter = 0;
    apu->buffer_position = 0;
}

// Calcul de la fréquence
u16 apu_calculate_frequency(u8 freq_lo, u8 freq_hi) {
    u16 freq = (freq_hi << 8) | freq_lo;
    return (2048 - freq) * 4; // Période en cycles
}

// Obtention du pattern de duty
u8 apu_get_duty_pattern(u8 duty) {
    return (duty >> 6) & 0x03;
}

// Mise à jour de l'envelope pour les canaux Square
void apu_update_envelope(SquareChannel* ch) {
    if (ch->envelope_period == 0) return;
    
    ch->envelope_timer--;
    if (ch->envelope_timer == 0) {
        ch->envelope_timer = ch->envelope_period;
        
        if (ch->envelope_increasing && ch->envelope_volume < 15) {
            ch->envelope_volume++;
        } else if (!ch->envelope_increasing && ch->envelope_volume > 0) {
            ch->envelope_volume--;
        }
        
        ch->volume = ch->envelope_volume;
    }
}

// Mise à jour de l'envelope pour le canal Noise
void apu_update_envelope_noise(NoiseChannel* ch) {
    if (ch->envelope_period == 0) return;
    
    ch->envelope_timer--;
    if (ch->envelope_timer == 0) {
        ch->envelope_timer = ch->envelope_period;
        
        if (ch->envelope_increasing && ch->envelope_volume < 15) {
            ch->envelope_volume++;
        } else if (!ch->envelope_increasing && ch->envelope_volume > 0) {
            ch->envelope_volume--;
        }
        
        ch->volume = ch->envelope_volume;
    }
}

// Mise à jour du compteur de longueur pour Square
void apu_update_length_counter(SquareChannel* ch) {
    if (ch->length_counter > 0) {
        ch->length_counter--;
        if (ch->length_counter == 0) {
            ch->enabled = false;
        }
    }
}

// Mise à jour du compteur de longueur pour Wave
void apu_update_length_counter_wave(WaveChannel* ch) {
    if (ch->length_counter > 0) {
        ch->length_counter--;
        if (ch->length_counter == 0) {
            ch->enabled = false;
        }
    }
}

// Mise à jour du compteur de longueur pour Noise
void apu_update_length_counter_noise(NoiseChannel* ch) {
    if (ch->length_counter > 0) {
        ch->length_counter--;
        if (ch->length_counter == 0) {
            ch->enabled = false;
        }
    }
}

// Tick du canal Square
void square_channel_tick(SquareChannel* ch, u8 cycles) {
    if (!ch->enabled || !ch->dac_enabled) return;
    
    ch->period_counter -= cycles;
    if (ch->period_counter <= 0) {
        ch->period_counter += ch->frequency;
        ch->duty_position = (ch->duty_position + 1) & 7;
    }
}

// Tick du canal Wave
void wave_channel_tick(WaveChannel* ch, u8 cycles) {
    if (!ch->enabled || !ch->dac_enabled) return;
    
    ch->period_counter -= cycles;
    if (ch->period_counter <= 0) {
        ch->period_counter += ch->frequency;
        ch->wave_position = (ch->wave_position + 1) & 31;
        
        // Lire l'échantillon depuis la wave RAM
        u8 wave_byte = ch->wave_ram[ch->wave_position / 2];
        if (ch->wave_position & 1) {
            ch->sample_buffer = wave_byte & 0x0F;
        } else {
            ch->sample_buffer = (wave_byte >> 4) & 0x0F;
        }
    }
}

// Tick du canal Noise
void noise_channel_tick(NoiseChannel* ch, u8 cycles) {
    if (!ch->enabled || !ch->dac_enabled) return;
    
    ch->period_counter -= cycles;
    if (ch->period_counter <= 0) {
        ch->period_counter += ch->frequency;
        
        // LFSR (Linear Feedback Shift Register)
        u16 feedback = (ch->lfsr ^ (ch->lfsr >> 1)) & 1;
        ch->lfsr = (ch->lfsr >> 1) | (feedback << 14);
        
        // Le bit 0 du LFSR détermine l'output
        ch->sample_buffer = (ch->lfsr & 1) ? 1 : 0;
    }
}

// Tick principal de l'APU
void apu_tick(APU* apu, u8 cycles) {
    if (!apu->apu_enabled) return;
    
    // Frame sequencer (512 Hz)
    apu->frame_sequencer += cycles;
    if (apu->frame_sequencer >= 8192) { // 4194304 / 512
        apu->frame_sequencer -= 8192;
        
        // Mise à jour des compteurs de longueur et d'envelope
        if (apu->frame_sequencer & 1) {
            // Longueur (256 Hz)
            apu_update_length_counter(&apu->square1);
            apu_update_length_counter(&apu->square2);
            apu_update_length_counter_wave(&apu->wave);
            apu_update_length_counter_noise(&apu->noise);
        }
        
        if ((apu->frame_sequencer & 3) == 2) {
            // Envelope (64 Hz)
            apu_update_envelope(&apu->square1);
            apu_update_envelope(&apu->square2);
            apu_update_envelope_noise(&apu->noise);
        }
    }
    
    // Tick des canaux
    square_channel_tick(&apu->square1, cycles);
    square_channel_tick(&apu->square2, cycles);
    wave_channel_tick(&apu->wave, cycles);
    noise_channel_tick(&apu->noise, cycles);
}

// Écriture dans les registres APU
void apu_write(APU* apu, u16 address, u8 value) {
    if (!apu->apu_enabled && address != NR52_REG) return;
    
    switch (address) {
        case NR10_REG: // Channel 1 Sweep
            apu->square1.sweep = value;
            break;
            
        case NR11_REG: // Channel 1 Duty/Length
            apu->square1.duty = value;
            apu->square1.duty_cycle = apu_get_duty_pattern(value);
            apu->square1.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR12_REG: // Channel 1 Envelope
            apu->square1.envelope = value;
            apu->square1.envelope_volume = (value >> 4) & 0x0F;
            apu->square1.envelope_increasing = (value & 0x08) != 0;
            apu->square1.envelope_period = value & 0x07;
            apu->square1.envelope_timer = apu->square1.envelope_period;
            apu->square1.dac_enabled = (value & 0xF8) != 0;
            break;
            
        case NR13_REG: // Channel 1 Frequency lo
            apu->square1.freq_lo = value;
            apu->square1.frequency = apu_calculate_frequency(apu->square1.freq_lo, apu->square1.freq_hi);
            break;
            
        case NR14_REG: // Channel 1 Frequency hi
            apu->square1.freq_hi = value;
            apu->square1.frequency = apu_calculate_frequency(apu->square1.freq_lo, apu->square1.freq_hi);
            if (value & 0x80) {
                // Trigger
                apu->square1.enabled = true;
                apu->square1.period_counter = apu->square1.frequency;
                apu->square1.duty_position = 0;
                apu->square1.volume = apu->square1.envelope_volume;
            }
            break;
            
        case NR21_REG: // Channel 2 Duty/Length
            apu->square2.duty = value;
            apu->square2.duty_cycle = apu_get_duty_pattern(value);
            apu->square2.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR22_REG: // Channel 2 Envelope
            apu->square2.envelope = value;
            apu->square2.envelope_volume = (value >> 4) & 0x0F;
            apu->square2.envelope_increasing = (value & 0x08) != 0;
            apu->square2.envelope_period = value & 0x07;
            apu->square2.envelope_timer = apu->square2.envelope_period;
            apu->square2.dac_enabled = (value & 0xF8) != 0;
            break;
            
        case NR23_REG: // Channel 2 Frequency lo
            apu->square2.freq_lo = value;
            apu->square2.frequency = apu_calculate_frequency(apu->square2.freq_lo, apu->square2.freq_hi);
            break;
            
        case NR24_REG: // Channel 2 Frequency hi
            apu->square2.freq_hi = value;
            apu->square2.frequency = apu_calculate_frequency(apu->square2.freq_lo, apu->square2.freq_hi);
            if (value & 0x80) {
                // Trigger
                apu->square2.enabled = true;
                apu->square2.period_counter = apu->square2.frequency;
                apu->square2.duty_position = 0;
                apu->square2.volume = apu->square2.envelope_volume;
            }
            break;
            
        case NR30_REG: // Channel 3 Enable
            apu->wave.enable = value;
            apu->wave.dac_enabled = (value & 0x80) != 0;
            break;
            
        case NR31_REG: // Channel 3 Length
            apu->wave.length = value;
            apu->wave.length_counter = 256 - value;
            break;
            
        case NR32_REG: // Channel 3 Output level
            apu->wave.output_level = value;
            break;
            
        case NR33_REG: // Channel 3 Frequency lo
            apu->wave.freq_lo = value;
            apu->wave.frequency = apu_calculate_frequency(apu->wave.freq_lo, apu->wave.freq_hi);
            break;
            
        case NR34_REG: // Channel 3 Frequency hi
            apu->wave.freq_hi = value;
            apu->wave.frequency = apu_calculate_frequency(apu->wave.freq_lo, apu->wave.freq_hi);
            if (value & 0x80) {
                // Trigger
                apu->wave.enabled = true;
                apu->wave.period_counter = apu->wave.frequency;
                apu->wave.wave_position = 0;
            }
            break;
            
        case NR41_REG: // Channel 4 Length
            apu->noise.length = value;
            apu->noise.length_counter = 64 - (value & 0x3F);
            break;
            
        case NR42_REG: // Channel 4 Envelope
            apu->noise.envelope = value;
            apu->noise.envelope_volume = (value >> 4) & 0x0F;
            apu->noise.envelope_increasing = (value & 0x08) != 0;
            apu->noise.envelope_period = value & 0x07;
            apu->noise.envelope_timer = apu->noise.envelope_period;
            apu->noise.dac_enabled = (value & 0xF8) != 0;
            break;
            
        case NR43_REG: // Channel 4 Polynomial
            apu->noise.polynomial = value;
            // Calcul de la fréquence pour le noise
            u8 shift = (value >> 4) & 0x0F;
            u8 divisor = value & 0x07;
            if (divisor == 0) divisor = 8;
            apu->noise.frequency = (divisor << shift) * 2;
            break;
            
        case NR44_REG: // Channel 4 Counter
            apu->noise.counter = value;
            if (value & 0x80) {
                // Trigger
                apu->noise.enabled = true;
                apu->noise.period_counter = apu->noise.frequency;
                apu->noise.lfsr = 0x7FFF; // Valeur initiale
                apu->noise.volume = apu->noise.envelope_volume;
            }
            break;
            
        case NR50_REG: // Master volume
            apu->nr50 = value;
            break;
            
        case NR51_REG: // Channel selection
            apu->nr51 = value;
            break;
            
        case NR52_REG: // Master enable
            apu->nr52 = value;
            apu->apu_enabled = (value & 0x80) != 0;
            if (!apu->apu_enabled) {
                // Désactiver tous les canaux
                apu->square1.enabled = false;
                apu->square2.enabled = false;
                apu->wave.enabled = false;
                apu->noise.enabled = false;
            }
            break;
            
        default:
            // Wave pattern RAM (0xFF30-0xFF3F)
            if (address >= WAVE_START && address <= WAVE_END) {
                apu->wave.wave_ram[address - WAVE_START] = value;
            }
            break;
    }
}

// Lecture des registres APU
u8 apu_read(APU* apu, u16 address) {
    if (!apu->apu_enabled && address != NR52_REG) return 0xFF;
    
    switch (address) {
        case NR10_REG:
            return apu->square1.sweep | 0x80;
        case NR11_REG:
            return apu->square1.duty | 0x3F;
        case NR12_REG:
            return apu->square1.envelope;
        case NR13_REG:
            return 0xFF;
        case NR14_REG:
            return apu->square1.freq_hi | 0xBF;
            
        case NR21_REG:
            return apu->square2.duty | 0x3F;
        case NR22_REG:
            return apu->square2.envelope;
        case NR23_REG:
            return 0xFF;
        case NR24_REG:
            return apu->square2.freq_hi | 0xBF;
            
        case NR30_REG:
            return apu->wave.enable | 0x7F;
        case NR31_REG:
            return 0xFF;
        case NR32_REG:
            return apu->wave.output_level | 0x9F;
        case NR33_REG:
            return 0xFF;
        case NR34_REG:
            return apu->wave.freq_hi | 0xBF;
            
        case NR41_REG:
            return 0xFF;
        case NR42_REG:
            return apu->noise.envelope;
        case NR43_REG:
            return apu->noise.polynomial;
        case NR44_REG:
            return apu->noise.counter | 0xBF;
            
        case NR50_REG:
            return apu->nr50;
        case NR51_REG:
            return apu->nr51;
        case NR52_REG:
            return apu->nr52 | 0x70;
            
        default:
            // Wave pattern RAM
            if (address >= WAVE_START && address <= WAVE_END) {
                return apu->wave.wave_ram[address - WAVE_START];
            }
            return 0xFF;
    }
}

// Mélange des canaux audio
void apu_mix_channels(APU* apu, s16* left, s16* right) {
    s16 left_sample = 0;
    s16 right_sample = 0;
    
    // Canal 1 (Square 1)
    if (apu->square1.enabled && apu->square1.dac_enabled) {
        s16 sample = DUTY_PATTERNS[apu->square1.duty_cycle][apu->square1.duty_position] * apu->square1.volume;
        if (apu->nr51 & 0x11) left_sample += sample;  // Left
        if (apu->nr51 & 0x22) right_sample += sample; // Right
    }
    
    // Canal 2 (Square 2)
    if (apu->square2.enabled && apu->square2.dac_enabled) {
        s16 sample = DUTY_PATTERNS[apu->square2.duty_cycle][apu->square2.duty_position] * apu->square2.volume;
        if (apu->nr51 & 0x12) left_sample += sample;  // Left
        if (apu->nr51 & 0x24) right_sample += sample; // Right
    }
    
    // Canal 3 (Wave)
    if (apu->wave.enabled && apu->wave.dac_enabled) {
        s16 sample = apu->wave.sample_buffer;
        u8 output_level = (apu->wave.output_level >> 5) & 0x03;
        if (output_level > 0) {
            sample >>= (4 - output_level);
        } else {
            sample = 0;
        }
        if (apu->nr51 & 0x14) left_sample += sample;  // Left
        if (apu->nr51 & 0x28) right_sample += sample; // Right
    }
    
    // Canal 4 (Noise)
    if (apu->noise.enabled && apu->noise.dac_enabled) {
        s16 sample = apu->noise.sample_buffer * apu->noise.volume;
        if (apu->nr51 & 0x18) left_sample += sample;  // Left
        if (apu->nr51 & 0x30) right_sample += sample; // Right
    }
    
    // Appliquer le volume master
    u8 left_volume = (apu->nr50 >> 4) & 0x07;
    u8 right_volume = apu->nr50 & 0x07;
    
    *left = (left_sample * left_volume) / 7;
    *right = (right_sample * right_volume) / 7;
}

// Rendu audio
void apu_render(APU* apu, s16* left, s16* right) {
    apu_mix_channels(apu, left, right);
}
