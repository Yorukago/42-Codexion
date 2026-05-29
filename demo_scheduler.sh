#!/usr/bin/env bash

# Compare FIFO vs EDF on the same hub parameters (invoked by `make demo`)
# Optional:
#   DEMO_PAUSE=1        — pause between big sections (make demo-walk)
#   DEMO_INTERACTIVE=1  — one keypress per log line (make demo-keys); reads from /dev/tty

set -u

CODEXION="${1:-./codexion}"
if [ ! -f "$CODEXION" ]; then
	printf 'demo_scheduler.sh: %s not found (run from repo root after make)\n' "$CODEXION" >&2
	exit 1
fi

DEMO_ARGS="4 2200 90 70 70 4 130"
DEMO_MAX_LINES="${DEMO_MAX_LINES:-32}"

DEMO_INTERACTIVE="${DEMO_INTERACTIVE:-0}"
INT=0
if [ "$DEMO_INTERACTIVE" = 1 ]; then
	if [ -r /dev/tty ]; then
		INT=1
	else
		printf 'demo: no controlling tty (open a real terminal for key-by-key)\n' >&2
		printf 'demo: falling back to non-interactive output\n\n' >&2
	fi
fi

step()
{
	printf '\n\033[1;36m━━ %s ━━\033[0m\n' "$1"
}

pause_step() 
{
	if [ "${DEMO_PAUSE:-0}" = 1 ]; then
		read -r -p "Press Enter to continue… " _ </dev/tty 2>/dev/null || read -r -p "Press Enter to continue… " _
	fi
}

print_move_line() 
{
	local line="$1"
	if [[ "$line" =~ ^([0-9]+)\ ([0-9]+)\ (.+)$ ]]; then
		printf '  %6s ms   Coder %s → %s\n' "${BASH_REMATCH[1]}" "${BASH_REMATCH[2]}" "${BASH_REMATCH[3]}"
	else
		printf '  %s\n' "$line"
	fi
}

spaced_moves() 
{
	local line
	while IFS= read -r line || [ -n "${line:-}" ]; do
		[ -z "$line" ] && continue
		print_move_line "$line"
	done
}

replay_moves() 
{
	local f="$1"
	local line
	local skip=0

	while IFS= read -r line || [ -n "${line:-}" ]; do
		[ -z "$line" ] && continue
		print_move_line "$line"

		if [ "$INT" != 1 ] || [ "$skip" = 1 ]; then
			continue
		fi

		printf '\033[90m  [any key → next move · q → show rest of this run · Q → quit demo]\033[0m  '
		if ! read -rsn1 key </dev/tty 2>/dev/null; then
			printf '\r\033[K\n'
			skip=1
			continue
		fi
		printf '\r\033[K'
		case "$key" in
			q) skip=1 ;;
			Q) DEMO_QUIT_ALL=1; break ;;
		esac
	done <"$f"
}

capture_and_show() 
{
	local pol="$1"
	local tmp

	tmp=$(mktemp)
	"$CODEXION" $DEMO_ARGS "$pol" 2>/dev/null | head -n "$DEMO_MAX_LINES" >"$tmp" || true

	if [ "$INT" = 1 ]; then
		replay_moves "$tmp"
	else
		spaced_moves <"$tmp"
	fi
	rm -f "$tmp"
}

DEMO_QUIT_ALL=0

step "Step 1 — Scenario (identical for FIFO and EDF)"
cat <<'EOF'
  Hub: 4 coders, 4 dongles (ring).
  Burnout clock: 2200 ms without *starting* a new compile since last compile start
                 (or since simulation start)
  Phases: compile 90 ms (two dongles held), debug 70 ms, refactor 70 ms
  Success stop: each coder reaches 4 compiles (or first burnout) — short trace on purpose
  Cooldown: 130 ms after a dongle is released → contention at shared dongles

  Only the last argument changes: fifo vs edf.
EOF
pause_step

step "Step 2 — Run with FIFO (queues by request arrival at each dongle)"
printf '  Command: %s %s fifo\n' "$CODEXION" "$DEMO_ARGS"
if [ "$INT" = 1 ]; then
	printf '  \033[1;33mKey mode:\033[0m each line appears after you press a key (Space is fine)\n'
fi
printf '\n'
capture_and_show fifo
printf '  … (captured at most %s lines; DEMO_MAX_LINES=60 for more)\n' "$DEMO_MAX_LINES"
pause_step

[ "${DEMO_QUIT_ALL:-0}" = 1 ] && exit 0

printf '\n\033[1;35m  ————  same hub, other scheduler  ————\033[0m\n'

step "Step 3 — Run with EDF (queues by nearest burnout deadline at each dongle)"
printf '  Command: %s %s edf\n' "$CODEXION" "$DEMO_ARGS"
if [ "$INT" = 1 ]; then
	printf '  \033[1;33mKey mode:\033[0m same as above — advance one simulator line per key\n'
fi
printf '\n'
capture_and_show edf
printf '  … (captured at most %s lines; DEMO_MAX_LINES=60 for more)\n' "$DEMO_MAX_LINES"
pause_step

step "Step 4 — What to look for"
cat <<'EOF'
  • Same hub; only fifo vs edf changes who tends to win when several coders wait on the
    same dongle after cooldown.

  • make demo        — prints all moves at once (one line per event, tight spacing)
  • make demo-keys   — one keypress per move (Space counts); needs a real TTY
  • make demo-walk   — Enter between big sections (DEMO_PAUSE=1)
  • DEMO_MAX_LINES=60 make demo
EOF
