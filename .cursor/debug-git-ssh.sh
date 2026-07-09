#!/usr/bin/env bash
# Git Bash 诊断脚本 — 写入 debug-1b583a.log
LOG_PATH="/d/gitHub/Aura/debug-1b583a.log"

log_json() {
  local hyp="$1" loc="$2" msg="$3" data="$4"
  local ts
  ts=$(date +%s000)
  printf '{"sessionId":"1b583a","runId":"baseline","hypothesisId":"%s","location":"%s","message":"%s","data":%s,"timestamp":%s}\n' \
    "$hyp" "$loc" "$msg" "$data" "$ts" >> "$LOG_PATH"
}

# region agent log
log_json "A" "debug-git-ssh.sh:env" "Git Bash 环境（终端可用）" \
  "{\"shell\":\"git-bash\",\"user\":\"$(whoami)\",\"gitPath\":\"$(command -v git)\",\"sshPath\":\"$(command -v ssh)\",\"SSH_AUTH_SOCK\":\"${SSH_AUTH_SOCK:-}\",\"SSH_AGENT_PID\":\"${SSH_AGENT_PID:-}\",\"cwd\":\"$(pwd)\"}"

AGENT_KEYS="$(ssh-add -l 2>&1 | tr '\n' ' ')"
log_json "A,E" "debug-git-ssh.sh:agent" "Git Bash ssh-agent 状态" \
  "{\"loadedKeys\":\"${AGENT_KEYS}\"}"

SSH_TEST="$(ssh -o BatchMode=yes -T git@github.com 2>&1 | tr '\n' ' ')"
log_json "B,C,E" "debug-git-ssh.sh:ssh-test" "ssh -T git@github.com 结果" \
  "{\"output\":\"${SSH_TEST}\"}"

REMOTE="$(git remote get-url origin 2>&1 | tr '\n' ' ')"
log_json "D" "debug-git-ssh.sh:remote" "git remote 配置" \
  "{\"originUrl\":\"${REMOTE}\"}"

PUSH_DRY="$(GIT_SSH_COMMAND='ssh -v' git push origin main --dry-run 2>&1 | tail -8 | tr '\n' ' ')"
log_json "B,C" "debug-git-ssh.sh:push-dry" "git push --dry-run 诊断" \
  "{\"tailOutput\":\"${PUSH_DRY}\"}"

log_json "ALL" "debug-git-ssh.sh:done" "Git Bash 诊断完成" "{\"ok\":true}"
# endregion

echo "诊断完成，日志已写入: $LOG_PATH"
