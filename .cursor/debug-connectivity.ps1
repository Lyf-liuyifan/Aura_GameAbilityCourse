$LogPath = "d:\gitHub\Aura\debug-1b583a.log"
$RunId = if ($env:DEBUG_RUN_ID) { $env:DEBUG_RUN_ID } else { "ssl-test" }

function Write-DebugLog {
    param([string]$HypothesisId, [string]$Message, [hashtable]$Data)
    $entry = @{
        sessionId    = "1b583a"
        runId        = $RunId
        hypothesisId = $HypothesisId
        location     = "debug-connectivity.ps1"
        message      = $Message
        data         = $Data
        timestamp    = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    } | ConvertTo-Json -Compress -Depth 6
    Add-Content -Path $LogPath -Value $entry -Encoding UTF8
}

function Test-GitCmd {
    param([string]$Label, [string]$HypothesisId, [string[]]$GitArgs)
    $out = & git @GitArgs 2>&1 | Out-String
    $trimmed = $out.Trim()
    if ($trimmed.Length -gt 300) { $trimmed = $trimmed.Substring(0, 300) }
    Write-DebugLog -HypothesisId $HypothesisId -Message $Label -Data @{
        exitCode = $LASTEXITCODE
        output   = $trimmed
    }
}

Write-DebugLog -HypothesisId "F,G" -Message "env" -Data @{
    sslBackend = (git config --global http.sslBackend 2>&1 | Out-String).Trim()
    httpProxy  = (git config --global http.proxy 2>&1 | Out-String).Trim()
    remoteUrl  = (git remote get-url origin 2>&1 | Out-String).Trim()
    sshCommand = (git config --global core.sshCommand 2>&1 | Out-String).Trim()
}

Test-GitCmd -Label "https-openssl" -HypothesisId "F" -GitArgs @(
    "-c", "http.sslBackend=openssl", "ls-remote", "https://github.com/Lyf-liuyifan/Aura_GameAbilityCourse.git"
)

Test-GitCmd -Label "https-schannel" -HypothesisId "F" -GitArgs @(
    "-c", "http.sslBackend=schannel", "ls-remote", "https://github.com/Lyf-liuyifan/Aura_GameAbilityCourse.git"
)

git remote set-url origin git@github.com:Lyf-liuyifan/Aura_GameAbilityCourse.git | Out-Null
Test-GitCmd -Label "ssh-ls-remote" -HypothesisId "H" -GitArgs @("ls-remote", "origin")

$agentKeys = (& ssh-add -l 2>&1 | Out-String).Trim()
Write-DebugLog -HypothesisId "A,E" -Message "ssh-agent-keys" -Data @{ loadedKeys = $agentKeys }
Write-DebugLog -HypothesisId "ALL" -Message "done" -Data @{ ok = $true }

Write-Host "Done"
