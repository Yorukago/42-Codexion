#!/bin/bash

# ===================================================================
# Codexion — evaluator test script
# Usage: make tests  (or ./tests.sh directly)
# ===================================================================

BINARY="./codexion"
RED="\033[1;31m"
GRN="\033[1;32m"
CYN="\033[1;36m"
YEL="\033[1;33m"
RST="\033[0m"

# Time limit (seconds) for each run to avoid hanging forever
TIMEOUT=10

pass=0
fail=0

run() {
	local label="$1"
	local expect="$2"   # "ok" | "burnout" | "error"
	shift 2
	local args="$@"

	printf "${CYN}  %-45s${RST}${YEL}%-30s${RST}" "$label" "$args"

	output=$(timeout $TIMEOUT $BINARY $args 2>&1)
	exit_code=$?

	if [ $exit_code -eq 124 ]; then
		printf "${RED}TIMEOUT${RST}\n"
		fail=$((fail + 1))
		return
	fi

	case "$expect" in
		ok)
			# Expect clean finish — no "burned out" line, exit 0
			if echo "$output" | grep -q "burned out"; then
				printf "${RED}FAIL${RST} (unexpected burnout)\n"
				fail=$((fail + 1))
			elif [ $exit_code -ne 0 ]; then
				printf "${RED}FAIL${RST} (exit $exit_code)\n"
				fail=$((fail + 1))
			else
				printf "${GRN}OK${RST}\n"
				pass=$((pass + 1))
			fi
			;;
		burnout)
			# Expect a "burned out" line to appear
			if echo "$output" | grep -q "burned out"; then
				printf "${GRN}OK${RST} (burnout detected)\n"
				pass=$((pass + 1))
			else
				printf "${RED}FAIL${RST} (no burnout)\n"
				fail=$((fail + 1))
			fi
			;;
		error)
			# Expect a non-zero exit (bad args)
			if [ $exit_code -ne 0 ]; then
				printf "${GRN}OK${RST} (rejected)\n"
				pass=$((pass + 1))
			else
				printf "${RED}FAIL${RST} (should have errored)\n"
				fail=$((fail + 1))
			fi
			;;
	esac
}

section() {
	printf "\n${YEL}── $1 ──${RST}\n"
}

echo ""
echo "  Codexion test suite"
echo "  binary: $BINARY"
echo ""

# -------------------------------------------------------------------
section "Invalid arguments"
# -------------------------------------------------------------------
# args: nb_coders burnout compile debug refactor compiles_required cooldown scheduler

run "no arguments"                          error
run "too few arguments (7 instead of 8)"    error  5 800 200 100 100 3 0
run "too many arguments (9 instead of 8)"   error  5 800 200 100 100 3 0 fifo extra
run "nb_coders = 0"                         error  0 800 200 100 100 3 0 fifo
run "nb_coders negative"                    error  -1 800 200 100 100 3 0 fifo
run "time_to_burnout = 0"                   error  5 0 200 100 100 3 0 fifo
run "nb_compiles_required = 0"              error  5 800 200 100 100 0 0 fifo
run "unknown scheduler"                     error  5 800 200 100 100 3 0 random
run "non-numeric argument"                  error  abc 800 200 100 100 3 0 fifo
run "scheduler uppercase (FIFO)"            error  5 800 200 100 100 3 0 FIFO
run "scheduler uppercase (EDF)"             error  5 800 200 100 100 3 0 EDF

# -------------------------------------------------------------------
section "Single coder"
# -------------------------------------------------------------------

run "1 coder, always burns out — fifo"      burnout  1 800 200 100 100 3 0 fifo
run "1 coder, always burns out — edf"       burnout  1 800 200 100 100 3 0 edf
run "1 coder, burnout (compile > burnout)"  burnout  1 100 200 50 50 5 0 fifo

# -------------------------------------------------------------------
section "Normal runs — FIFO"
# -------------------------------------------------------------------

run "2 coders, 3 compiles"                  ok       2 2000 200 100 100 3 0 fifo
run "3 coders, 3 compiles"                  ok       3 2000 200 100 100 3 0 fifo
run "4 coders, 3 compiles"                  ok       4 2000 200 100 100 3 0 fifo
run "5 coders, 5 compiles"                  ok       5 2000 200 100 100 5 0 fifo

# -------------------------------------------------------------------
section "Normal runs — EDF"
# -------------------------------------------------------------------

run "2 coders, 3 compiles"                  ok       2 2000 200 100 100 3 0 edf
run "3 coders, 3 compiles"                  ok       3 2000 200 100 100 3 0 edf
run "4 coders, 3 compiles"                  ok       4 2000 200 100 100 3 0 edf
run "5 coders, 5 compiles"                  ok       5 2000 200 100 100 5 0 edf

# -------------------------------------------------------------------
section "Dongle cooldown"
# -------------------------------------------------------------------

run "cooldown 0 — no delay"                 ok       3 2000 200 100 100 3 0   fifo
run "cooldown 50ms — should still work"     ok       3 2000 200 100 100 3 50  fifo
run "cooldown 100ms — should still work"    ok       3 2000 200 100 100 3 100 edf

# -------------------------------------------------------------------
section "Burnout cases"
# -------------------------------------------------------------------

run "burnout: compile time > burnout time"  burnout  3 100 200 50 50 5 0 fifo
run "burnout: total cycle > burnout"        burnout  4 300 200 200 200 5 0 fifo
run "burnout: impossible cooldown"          burnout  2 300 100 50 50 5 400 fifo

# -------------------------------------------------------------------
section "Edge: large number of coders"
# -------------------------------------------------------------------

run "10 coders, 2 compiles — fifo"          ok       10 2000 200 100 100 2 0 fifo
run "10 coders, 2 compiles — edf"           ok       10 2000 200 100 100 2 0 edf

# -------------------------------------------------------------------
section "Edge: zero compile / debug / refactor time"
# -------------------------------------------------------------------

run "compile=0, debug=0, refactor=0 — fifo" ok       3 500 0 0 0 5 0 fifo
run "compile=0, debug=0, refactor=0 — edf"  ok       3 500 0 0 0 5 0 edf

# -------------------------------------------------------------------

echo ""
printf "  Results: ${GRN}$pass passed${RST}, ${RED}$fail failed${RST}\n\n"

[ $fail -eq 0 ] && exit 0 || exit 1