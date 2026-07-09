# Load SSH key into Windows ssh-agent (for Cursor UI git push)
# Run this in PowerShell BEFORE using Cursor sync button.
$LogPath = "d:\gitHub\Aura\debug-1b583a.log"
$RunId = if ($env:DEBUG_RUN_ID) { $env:DEBUG_RUN_ID } else { "load-ssh" }

function Write-DebugLog {
    param([string]$HypothesisId, [string]$Message, [hashtable]$Data)
    $entry = @{
        sessionId    = "1b583a"
        runId        = $RunId
        hypothesisId = $HypothesisId
        location     = "load-windows-ssh.ps1"
        message      = $Message
        data         = $Data
        timestamp    = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    } | ConvertTo-Json -Compress -Depth 6
    Add-Content -Path $LogPath -Value $entry -Encoding UTF8
}

# region agent log
$service = Get-Service ssh-agent -ErrorAction SilentlyContinue
if ($service -and $service.Status -ne "Running") {
    Start-Service ssh-agent
}

$before = (& ssh-add -l 2>&1 | Out-String).Trim()
Write-DebugLog -HypothesisId "A,E" -Message "before-ssh-add" -Data @{
    agentStatus = if ($service) { (Get-Service ssh-agent).Status.ToString() } else { "missing" }
    loadedKeys  = $before
}

if ($before -match "The agent has no identities|could not open a connection") {
    Write-Host "Enter passphrase for C:\Users\liu\.ssh\test ..."
    & ssh-add C:\Users\liu\.ssh\test
}

$after = (& ssh-add -l 2>&1 | Out-String).Trim()
Write-DebugLog -HypothesisId "A,E,H" -Message "after-ssh-add" -Data @{ loadedKeys = $after }

& git remote set-url origin git@github.com:Lyf-liuyifan/Aura_GameAbilityCourse.git | Out-Null
$test = (& git ls-remote origin 2>&1 | Out-String).Trim()
$ok = $LASTEXITCODE -eq 0
Write-DebugLog -HypothesisId "H" -Message "git-ls-remote" -Data @{
    exitCode = $LASTEXITCODE
    output   = if ($test.Length -gt 200) { $test.Substring(0, 200) } else { $test }
}
Write-DebugLog -HypothesisId "ALL" -Message "load-done" -Data @{ ok = $ok }
# endregion

if ($ok) {
    Write-Host "OK: Windows ssh-agent ready. Cursor sync should work now."
} else {
    Write-Host "FAIL: git ls-remote failed. Check output above."
    exit 1
}
