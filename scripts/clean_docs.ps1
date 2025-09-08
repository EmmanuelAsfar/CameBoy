$ErrorActionPreference='Stop'
if (Test-Path 'docs\specs\updates') { Remove-Item -Recurse -Force 'docs\specs\updates' }

$mdFiles = Get-ChildItem -Recurse -File docs -Filter *.md | Select-Object -ExpandProperty FullName

Add-Type -AssemblyName 'System.Runtime'
function Normalize-Ascii([string]$s){
  if ($null -eq $s) { return '' }
  $s = $s -replace '```mermaid','```'
  $s = $s.Replace([char]0x2013,'-').Replace([char]0x2014,'-')
  $s = $s.Replace([char]0x2018,'''').Replace([char]0x2019,'''')
  $s = $s.Replace([char]0x201C,'"').Replace([char]0x201D,'"')
  $s = $s.Replace([char]0x00A0,' ')
  $formD = $s.Normalize([Text.NormalizationForm]::FormD)
  $sb = New-Object System.Text.StringBuilder
  foreach($ch in $formD.ToCharArray()){
    $uc = [Globalization.CharUnicodeInfo]::GetUnicodeCategory($ch)
    if($uc -ne [Globalization.UnicodeCategory]::NonSpacingMark -and [int][char]$ch -ne 0xFFFD){
      [void]$sb.Append($ch)
    }
  }
  $plain = $sb.ToString().Normalize([Text.NormalizationForm]::FormC)
  $outSb = New-Object System.Text.StringBuilder
  foreach($ch in $plain.ToCharArray()){
    $code = [int][char]$ch
    if($code -in 9,10,13 -or ($code -ge 32 -and $code -le 126)){
      [void]$outSb.Append($ch)
    } elseif ($code -eq 160) { [void]$outSb.Append(' ') }
  }
  return $outSb.ToString()
}

foreach($f in $mdFiles){
  $raw = Get-Content -LiteralPath $f -Raw
  $clean = Normalize-Ascii $raw
  [System.IO.File]::WriteAllText($f, $clean, [System.Text.Encoding]::UTF8)
}

$nonAscii = @()
foreach($f in $mdFiles){
  $txt = Get-Content -LiteralPath $f -Raw
  if($txt -match '[^\u0009\u000A\u000D\u0020-\u007E]'){
    $nonAscii += $f
  }
}
Write-Output 'CLEAN_DONE'
if($nonAscii.Count -gt 0){
  Write-Output 'NON_ASCII_LEFT:'
  $nonAscii | ForEach-Object { Write-Output $_ }
}
