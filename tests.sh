#!/usr/bin/env bash
# ===================================================================
# Codexion — test suite
# Usage: make tests  (or ./tests.sh directly)
# ===================================================================

BINARY="./codexion"
RED="\033[1;31m"
GRN="\033[1;32m"
CYN="\033[1;36m"
YEL="\033[1;33m"
RST="\033[0m"
TIMEOUT=15

# Every non-empty output line must match this or the run is malformed
VALID='^[0-9]+ [0-9]+ (has taken a dongle|is compiling|is debugging|is refactoring|burned out)$'

pass=0
fail=0

run() {
	local label="$1" expect="$2"
	shift 2

	printf "${CYN}  %-50s${RST}${YEL}%-35s${RST}" "$label" "$*"

	output=$(timeout $TIMEOUT $BINARY "$@" 2>&1)
	exit_code=$?

	if [ $exit_code -eq 124 ]; then
		printf "${RED}TIMEOUT${RST}\n"; fail=$((fail + 1)); return
	fi

	local malformed
	malformed=$(echo "$output" | grep -v '^$' | grep -cvE "$VALID" || true)

	case "$expect" in
		ok)
			if echo "$output" | grep -q "burned out"; then
				printf "${RED}FAIL${RST} (unexpected burnout)\n"; fail=$((fail + 1))
			elif [ $exit_code -ne 0 ]; then
				printf "${RED}FAIL${RST} (exit $exit_code)\n"; fail=$((fail + 1))
			elif [ "$malformed" -gt 0 ]; then
				printf "${RED}FAIL${RST} (malformed output)\n"; fail=$((fail + 1))
			else
				printf "${GRN}OK${RST}\n"; pass=$((pass + 1))
			fi
			;;
		burnout)
			if ! echo "$output" | grep -q "burned out"; then
				printf "${RED}FAIL${RST} (no burnout)\n"; fail=$((fail + 1))
			elif [ "$malformed" -gt 0 ]; then
				printf "${RED}FAIL${RST} (malformed output)\n"; fail=$((fail + 1))
			else
				printf "${GRN}OK${RST} (burnout detected)\n"; pass=$((pass + 1))
			fi
			;;
		error)
			if [ $exit_code -ne 0 ]; then
				printf "${GRN}OK${RST} (rejected)\n"; pass=$((pass + 1))
			else
				printf "${RED}FAIL${RST} (should have errored, got exit 0)\n"; fail=$((fail + 1))
			fi
			;;
	esac
}

section() { printf "\n${YEL}── $1 ──${RST}\n"; }

echo ""
echo "  Codexion test suite"
echo "  binary: $BINARY"
echo ""

# -------------------------------------------------------------------
section "Invalid arguments"
# -------------------------------------------------------------------
# args: nb_coders burnout compile debug refactor compiles_required cooldown scheduler

run "no arguments"                           error
run "too few (7 args)"                       error  5 800 200 100 100 3 0
run "too many (9 args)"                      error  5 800 200 100 100 3 0 fifo extra
run "nb_coders = 0"                          error  0 800 200 100 100 3 0 fifo
run "nb_coders negative"                     error  -1 800 200 100 100 3 0 fifo
run "time_to_burnout = 0"                    error  5 0 200 100 100 3 0 fifo
run "time_to_burnout negative"               error  5 -800 200 100 100 3 0 fifo
run "nb_compiles_required = 0"               error  5 800 200 100 100 0 0 fifo
run "negative compile time"                  error  5 800 -200 100 100 3 0 fifo
run "negative cooldown"                      error  5 800 200 100 100 3 -5 fifo
run "float argument"                         error  5 800 200.5 100 100 3 0 fifo
run "leading + sign"                         error  5 800 +200 100 100 3 0 fifo
run "non-numeric (abc)"                      error  abc 800 200 100 100 3 0 fifo
run "INT_MAX + 1 (2147483648)"               error  5 2147483648 200 100 100 3 0 fifo
run "massive overflow"                       error  5 99999999999999999999 200 100 100 3 0 fifo
run "unknown scheduler"                      error  5 800 200 100 100 3 0 random
run "scheduler typo (efd)"                   error  5 800 200 100 100 3 0 efd
run "scheduler uppercase (FIFO)"             error  5 800 200 100 100 3 0 FIFO
run "scheduler uppercase (EDF)"              error  5 800 200 100 100 3 0 EDF
run "empty string as scheduler"              error  5 800 200 100 100 3 0 ""

# -------------------------------------------------------------------
section "Single coder — always burns out (left == right dongle, never acquires 2)"
# -------------------------------------------------------------------

run "1 coder — fifo"                         burnout  1 800 200 100 100 3 0 fifo
run "1 coder — edf"                          burnout  1 800 200 100 100 3 0 edf
run "1 coder, only 1 compile required"       burnout  1 800 100 50 50 1 0 fifo
run "1 coder, burnout=3s, still burns out"   burnout  1 3000 1 1 1 5 0 fifo

# -------------------------------------------------------------------
section "Normal runs — FIFO"
# -------------------------------------------------------------------

run "2 coders, 3 compiles"                   ok  2 2000 200 100 100 3 0 fifo
run "3 coders, 3 compiles"                   ok  3 2000 200 100 100 3 0 fifo
run "4 coders, 3 compiles"                   ok  4 2000 200 100 100 3 0 fifo
run "5 coders, 5 compiles"                   ok  5 2000 200 100 100 5 0 fifo
run "all-zero cycle times, big burnout"      ok  4 5000 0 0 0 10 0 fifo

# -------------------------------------------------------------------
section "Normal runs — EDF"
# -------------------------------------------------------------------

run "2 coders, 3 compiles"                   ok  2 2000 200 100 100 3 0 edf
run "3 coders, 3 compiles"                   ok  3 2000 200 100 100 3 0 edf
run "4 coders, 3 compiles"                   ok  4 2000 200 100 100 3 0 edf
run "5 coders, 5 compiles"                   ok  5 2000 200 100 100 5 0 edf
run "all-zero cycle times, big burnout"      ok  4 5000 0 0 0 10 0 edf

# -------------------------------------------------------------------
section "Guaranteed burnouts — clear margins, no timing ambiguity"
# -------------------------------------------------------------------

# compile time alone exceeds burnout
run "compile > burnout (3 coders, fifo)"     burnout  3 100 200 0 0 5 0 fifo
run "compile > burnout (5 coders, edf)"      burnout  5 100 200 0 0 5 0 edf
# cooldown alone exceeds burnout — no way out
run "cooldown > burnout (2 coders)"          burnout  2 150 0 0 0 5 200 fifo
run "cooldown > burnout (3 coders, edf)"     burnout  3 100 0 0 0 5 200 edf
# full cycle (100+100+100=300) well over burnout (200), with margin
run "full cycle > burnout (4 coders)"        burnout  4 200 100 100 100 5 0 fifo
# cooldown dwarfs burnout regardless of scheduler
run "huge cooldown vs burnout — fifo"        burnout  5 200 50 50 50 10 300 fifo
run "huge cooldown vs burnout — edf"         burnout  5 200 50 50 50 10 300 edf

# -------------------------------------------------------------------
section "Dongle cooldown — comfortable margins, should not burnout"
# -------------------------------------------------------------------

run "cooldown 0"                             ok  3 2000 200 100 100 3 0   fifo
run "cooldown 50ms, burnout 3000ms"          ok  4 3000 200 100 100 3 50  fifo
run "cooldown 100ms, burnout 4000ms"         ok  4 4000 200 100 100 3 100 edf
run "cooldown 200ms, burnout 5000ms"         ok  3 5000 100 100 100 3 200 fifo

# -------------------------------------------------------------------
section "Stress — timed to finish well within the timeout"
# -------------------------------------------------------------------
# 10 coders × 5 compiles, cycle=100ms: ~1s
run "10 coders, 5 compiles — fifo"           ok  10 2000 50 25 25 5 0 fifo
run "10 coders, 5 compiles — edf"            ok  10 2000 50 25 25 5 0 edf
# 5 coders × 20 compiles, cycle=100ms: ~5s
run "5 coders, 20 compiles — fifo"           ok  5 2000 50 25 25 20 0 fifo
run "5 coders, 20 compiles — edf"            ok  5 2000 50 25 25 20 0 edf
# zero-time cycle: finishes near-instantly regardless of count
run "4 coders, 50 compiles, zero times"      ok  4 5000 0 0 0 50 0 fifo

# -------------------------------------------------------------------
section "Scheduler parity — same params must give same outcome"
# -------------------------------------------------------------------

run "feasible — fifo"                        ok       3 5000 100 100 100 5 0 fifo
run "feasible — edf"                         ok       3 5000 100 100 100 5 0 edf
run "impossible compile time — fifo"         burnout  3 50 200 0 0 5 0 fifo
run "impossible compile time — edf"          burnout  3 50 200 0 0 5 0 edf

echo ""
printf "  Results: ${GRN}$pass passed${RST}, ${RED}$fail failed${RST}\n\n"

[ $fail -eq 0 ] && exit 0 || exit 1
