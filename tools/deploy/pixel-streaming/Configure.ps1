param(
    [ValidateSet('Local','Public')][string]$Mode='Local',
    [string]$TurnHost,
    [string]$TurnPublicIP,
    [ValidateRange(1024,65535)][int]$WebPort=8080,
    [ValidateRange(1024,65535)][int]$StreamerPort=8888,
    [ValidateRange(1024,65535)][int]$DownloadPort=8081
)
$ErrorActionPreference='Stop'
$runtime=Join-Path $PSScriptRoot 'runtime'
if(@($WebPort,$StreamerPort,$DownloadPort | Select-Object -Unique).Count -ne 3){throw 'WebPort, StreamerPort and DownloadPort must differ.'}
if($Mode -eq 'Public') {
    if($TurnHost -notmatch '^[a-zA-Z0-9.-]+$' -or [string]::IsNullOrWhiteSpace($TurnPublicIP)) {
        throw 'Public mode requires -TurnHost (hostname or IPv4) and -TurnPublicIP (public IPv4 of the TURN server).'
    }
    $parsedIP=$null
    if(-not [Net.IPAddress]::TryParse($TurnPublicIP,[ref]$parsedIP) -or $parsedIP.AddressFamily -ne [Net.Sockets.AddressFamily]::InterNetwork) {
        throw 'TurnPublicIP must be a public IPv4 address.'
    }
}
New-Item -ItemType Directory -Force $runtime | Out-Null
$config=[ordered]@{
    player_port=8080; streamer_port=8888; max_players=1; serve=$true
    http_root='/opt/pixel-streaming/SignallingWebServer/www'; homepage='player.html'
    log_config=$false; rest_api=$false; log_level_console='info'; log_level_file='info'
    peer_options=@{iceServers=@()}
}
if($Mode -eq 'Public') {
    $secretPath=Join-Path $runtime 'turn-secret.txt'
    if(-not (Test-Path -LiteralPath $secretPath)) {
        $bytes=New-Object byte[] 32
        $rng=[Security.Cryptography.RandomNumberGenerator]::Create()
        try { $rng.GetBytes($bytes) } finally { $rng.Dispose() }
        [IO.File]::WriteAllText($secretPath,([BitConverter]::ToString($bytes).Replace('-','').ToLowerInvariant()))
    }
    $secret=[IO.File]::ReadAllText($secretPath).Trim()
    if($secret -notmatch '^[a-f0-9]{64}$'){throw 'Unexpected TURN secret format.'}
    $config.peer_options=@{
        iceTransportPolicy='relay'
        iceServers=@(@{urls=@("turn:${TurnHost}:3478?transport=udp","turn:${TurnHost}:3478?transport=tcp")})
    }
    $config.turn_secret_file='/config/turn-secret.txt'
    $turn=@"
listening-port=3478
listening-ip=0.0.0.0
external-ip=$TurnPublicIP
min-port=49160
max-port=49200
fingerprint
use-auth-secret
static-auth-secret=$secret
realm=$TurnHost
no-cli
no-multicast-peers
no-tls
no-dtls
log-file=stdout
"@
    [IO.File]::WriteAllText((Join-Path $runtime 'turnserver.conf'),$turn.Replace("`r`n","`n")+"`n")
}
[IO.File]::WriteAllText((Join-Path $runtime 'signalling.json'),($config | ConvertTo-Json -Depth 8))
[IO.File]::WriteAllText((Join-Path $PSScriptRoot '.env'),"WEB_PORT=$WebPort`nSTREAMER_PORT=$StreamerPort`nDOWNLOAD_PORT=$DownloadPort`n")
[IO.File]::WriteAllText((Join-Path $runtime 'host.json'),(@{webPort=$WebPort;streamerPort=$StreamerPort} | ConvertTo-Json))
Write-Host "Configured $Mode mode. Runtime configuration: $runtime"
if($Mode -eq 'Public'){Write-Host 'Copy runtime/turnserver.conf to the public TURN host alongside public-turn/compose.yaml. Keep the shared secret private.'}
