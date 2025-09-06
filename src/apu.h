#ifndef APU_H
#define APU_H

#include "common.h"

// Registres audio (0xFF10-0xFF3F)
#define NR10_REG 0xFF10  // Channel 1 Sweep
#define NR11_REG 0xFF11  // Channel 1 Sound length/Wave pattern duty
#define NR12_REG 0xFF12  // Channel 1 Volume Envelope
#define NR13_REG 0xFF13  // Channel 1 Frequency lo
#define NR14_REG 0xFF14  // Channel 1 Frequency hi
#define NR21_REG 0xFF16  // Channel 2 Sound length/Wave pattern duty
#define NR22_REG 0xFF17  // Channel 2 Volume Envelope
#define NR23_REG 0xFF18  // Channel 2 Frequency lo
#define NR24_REG 0xFF19  // Channel 2 Frequency hi
#define NR30_REG 0xFF1A  // Channel 3 Sound on/off
#define NR31_REG 0xFF1B  // Channel 3 Sound length
#define NR32_REG 0xFF1C  // Channel 3 Select output level
#define NR33_REG 0xFF1D  // Channel 3 Frequency lo
#define NR34_REG 0xFF1E  // Channel 3 Frequency hi
#define NR41_REG 0xFF20  // Channel 4 Sound length
#define NR42_REG 0xFF21  // Channel 4 Volume Envelope
#define NR43_REG 0xFF22  // Channel 4 Polynomial counter
#define NR44_REG 0xFF23  // Channel 4 Counter/Consecutive
#define NR50_REG 0xFF24  // Channel control / ON-OFF / Volume
#define NR51_REG 0xFF25  // Selection of Sound output terminal
#define NR52_REG 0xFF26  // Sound on/off
#define WAVE_START 0xFF30  // Wave pattern RAM start
#define WAVE_END   0xFF3F  // Wave pattern RAM end

// Canaux audio
typedef enum {
    CHANNEL_1 = 0,  // Square 1 (avec sweep)
    CHANNEL_2 = 1,  // Square 2
    CHANNEL_3 = 2,  // Wave
    CHANNEL_4 = 3   // Noise
} AudioChannel;

// Structure du canal Square (Channel 1 & 2)
typedef struct {
    // Registres
    u8 sweep;       // NR10/NR11 (selon canal)
    u8 duty;        // NR11/NR21
    u8 envelope;    // NR12/NR22
    u8 freq_lo;     // NR13/NR23
    u8 freq_hi;     // NR14/NR24
    
    // État interne
    u16 frequency;      // Fréquence calculée
    u16 period_counter; // Compteur de période
    u8 duty_cycle;      // Cycle de duty (0-3)
    u8 duty_position;   // Position dans le cycle
    u8 volume;          // Volume actuel
    u8 envelope_volume; // Volume de l'envelope
    u8 envelope_period; // Période de l'envelope
    u8 envelope_timer;  // Timer de l'envelope
    bool envelope_increasing; // Direction de l'envelope
    u16 length_counter; // Compteur de longueur
    bool enabled;       // Canal activé
    bool dac_enabled;   // DAC activé
} SquareChannel;

// Structure du canal Wave (Channel 3)
typedef struct {
    // Registres
    u8 enable;      // NR30
    u8 length;      // NR31
    u8 output_level; // NR32
    u8 freq_lo;     // NR33
    u8 freq_hi;     // NR34
    
    // Wave pattern RAM (32 bytes)
    u8 wave_ram[32];
    
    // État interne
    u16 frequency;      // Fréquence calculée
    u16 period_counter; // Compteur de période
    u16 length_counter; // Compteur de longueur
    u8 wave_position;   // Position dans la wave
    u8 sample_buffer;   // Buffer d'échantillon
    bool enabled;       // Canal activé
    bool dac_enabled;   // DAC activé
} WaveChannel;

// Structure du canal Noise (Channel 4)
typedef struct {
    // Registres
    u8 length;      // NR41
    u8 envelope;    // NR42
    u8 polynomial;  // NR43
    u8 counter;     // NR44
    
    // État interne
    u16 lfsr;           // Linear Feedback Shift Register
    u16 frequency;      // Fréquence calculée
    u16 period_counter; // Compteur de période
    u8 volume;          // Volume actuel
    u8 envelope_volume; // Volume de l'envelope
    u8 envelope_period; // Période de l'envelope
    u8 envelope_timer;  // Timer de l'envelope
    bool envelope_increasing; // Direction de l'envelope
    u16 length_counter; // Compteur de longueur
    u8 sample_buffer;   // Buffer d'échantillon
    bool enabled;       // Canal activé
    bool dac_enabled;   // DAC activé
} NoiseChannel;

// Structure principale de l'APU
typedef struct {
    // Registres de contrôle
    u8 nr50;  // Channel control / ON-OFF / Volume
    u8 nr51;  // Selection of Sound output terminal
    u8 nr52;  // Sound on/off
    
    // Canaux audio
    SquareChannel square1;
    SquareChannel square2;
    WaveChannel wave;
    NoiseChannel noise;
    
    // État global
    bool apu_enabled;     // APU activé
    u32 frame_sequencer;  // Compteur du frame sequencer
    u32 sample_rate;      // Taux d'échantillonnage
    u32 sample_counter;   // Compteur d'échantillons
    
    // Buffer audio
    s16* audio_buffer;
    u32 buffer_size;
    u32 buffer_position;
} APU;

// Fonctions APU
void apu_init(APU* apu);
void apu_cleanup(APU* apu);
void apu_reset(APU* apu);
void apu_tick(APU* apu, u8 cycles);
void apu_write(APU* apu, u16 address, u8 value);
u8 apu_read(APU* apu, u16 address);

// Rendu audio
void apu_render(APU* apu, s16* left, s16* right);
void apu_mix_channels(APU* apu, s16* left, s16* right);

// Gestion des canaux
void square_channel_tick(SquareChannel* ch, u8 cycles);
void wave_channel_tick(WaveChannel* ch, u8 cycles);
void noise_channel_tick(NoiseChannel* ch, u8 cycles);

// Utilitaires
u16 apu_calculate_frequency(u8 freq_lo, u8 freq_hi);
u8 apu_get_duty_pattern(u8 duty);
void apu_update_envelope(SquareChannel* ch);
void apu_update_envelope_noise(NoiseChannel* ch);
void apu_update_length_counter(SquareChannel* ch);
void apu_update_length_counter_wave(WaveChannel* ch);
void apu_update_length_counter_noise(NoiseChannel* ch);

#endif // APU_H
