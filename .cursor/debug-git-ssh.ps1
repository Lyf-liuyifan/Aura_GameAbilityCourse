# Cursor Git SSH 诊断脚本 — 写入 debug-1b583a.log
$LogPath = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) "debug-1b583a.log"
if (-not (Test-Path (Split-Path $LogPath -Parent))) {
    $LogPath = Join-Path $PSScriptRoot "..\debug-1b583a.log" | Resolve-Path -ErrorAction SilentlyContinue
}
$LogPath = "d:\gitHub\Aura\debug-1b583a.log"

function Write-DebugLog {
    param(
        [string]$HypothesisId,
        [string]$Location,
        [string]$Message,
        [hashtable]$Data,
        [string]$RunId = "baseline"
    )
    $entry = @{
        sessionId    = "1b583a"
        runId        = $RunId
        hypothesisId = $HypothesisId
        location     = $Location
        message      = $Message
        data         = $Data
        timestamp    = [DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds()
    } | ConvertTo-Json -Compress -Depth 6
    Add-Content -Path $LogPath -Value $entry -Encoding UTF8
}

# region agent log
Write-DebugLog -HypothesisId "A" -Location "debug-git-ssh.ps1:env" -Message "PowerShell 环境（模拟 Cursor UI）" -Data @{
    shell        = "powershell"
    user         = $env:USERNAME
    gitPath      = (Get-Command git -ErrorAction SilentlyContinue).Source
    sshPath      = (Get-Command ssh -ErrorAction SilentlyContinue).Source
    gitSsh       = $env:GIT_SSH
    gitSshCommand = $env:GIT_SSH_COMMAND
    sshAuthSock  = $env:SSH_AUTH_SOCK
    sshAgentPid  = $env:SSH_AGENT_PID
    cwd          = (Get-Location).Path
}

$agentService = Get-Service ssh-agent -ErrorAction SilentlyContinue
$agentKeys = & ssh-add -l 2>&1 | Out-String
Write-DebugLog -HypothesisId "A,E" -Location "debug-git-ssh.ps1:agent" -Message "Windows ssh-agent 状态" -Data @{
    agentStatus  = if ($agentService) { $agentService.Status.ToString() } else { "missing" }
    agentStartType = if ($agentService) { $agentService.StartType.ToString() } else { "missing" }
    loadedKeys   = $agentKeys.Trim()
}

$sshTest = & ssh -o BatchMode=yes -o ConnectTimeout=10 -T git@github.com 2>&1 | Out-String
Write-DebugLog -HypothesisId "B,C,E" -Location "debug-git-ssh.ps1:ssh-test" -Message "ssh -T git@github.com 结果" -Data @{
    exitCode = $LASTEXITCODE
    output   = $sshTest.Trim()
}

$gitRemote = & git remote get-url origin 2>&1 | Out-String
Write-DebugLog -HypothesisId "D" -Location "debug-git-ssh.ps1:remote" -Message "git remote 配置" -Data @{
    originUrl = $gitRemote.Trim()
    sshConfigExists = Test-Path "$env:USERPROFILE\.ssh\config"
    identityFile = if (Test-Path "$env:USERPROFILE\.ssh\config") {
        (Select-String -Path "$env:USERPROFILE\.ssh\config" -Pattern "IdentityFile" | ForEach-Object { $_.Line }) -join "; "
    } else { "none" }
}

$pushDry = & git -c core.sshCommand="ssh -v" push origin main --dry-run 2>&1 | Out-String
$pushFailReason = if ($pushDry -match "read_passphrase") { "passphrase_no_tty" }
                  elseif ($pushDry -match "Permission denied \(publickey\)") { "publickey_denied" }
                  elseif ($pushDry -match "Could not read from remote") { "remote_read_failed" }
                  else { "other" }
Write-DebugLog -HypothesisId "B,C" -Location "debug-git-ssh.ps1:push-dry" -Message "git push --dry-run 诊断" -Data @{
    failReason = $pushFailReason
    tailOutput = ($pushDry -split "`n" | Select-Object -Last 8) -join " | "
}

Write-DebugLog -HypothesisId "ALL" -Location "debug-git-ssh.ps1:done" -Message "PowerShell 诊断完成" -Data @{ ok = $true; remote = (& git remote get-url origin 2>&1 | Out-String).Trim() } -RunId $env:DEBUG_RUN_ID
# endregion

Write-Host "诊断完成，日志已写入: $LogPath"
