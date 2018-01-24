#!/bin/bash
# test how card responds to  various APDUs  to find applications
#ISO 7816-4  Section  8 Application-independent card services
#
# want to test for duplicate AIDs, PIV and OpenPGP etc. 
#

CMD="/usr/local/bin/opensc-tool -c default"

# PIV SELECT AID for PIV + Le=256
PIV=(-s "00 A4 04 00 09 A0 00 00 03 08 00 00 10 00 00")

# OpenPGP AID + Le=256
PGP=(-s "00 A4 04 00 06 D2 76 00 01 24 01 00")

# muscle AID + Le=256 Used ti try aa AID that is not on the card
MUSCLE=(-s "00 A4 04 00 06 A0 00 00 00 01 01 00")

# read EF.DIR  3F002F00 using select file Select from the MF
REFDIR=(-s "00 A4 08 00 02 2F 00 00" -s "00 B0 00 00 1F")

# Try GET DATA EF.DIR Use first if known to work
#GEFDIR=(-s "00 A4 08 00 02 2F 00 00" -s "00 CA 00 00 00 00")
GEFDIR=(-s "00 CA 00 00 02 5C 00 00")

#Verify PIV pin 123456
V80P=(-s "00 20 00 80 08 31 32 33 34 35 36 FF FF")
#VERIFY PIV login state i.e. VERIFY with Lc absent
V800=(-s "00 20 00 80")

#Verify PGP pin 222222 for the login pin 82
V82P=(-s "00 20 00 82 06 32 32 32 32 32 32")
#VERIFY PGP login state
V820=(-s "00 20 00 82")
#
set -x 

while test $# -gt 0 ; do
arg="$1"
  case $arg in 
    piv.ef)
	echo  Select AID, try and read EF.DIR, not expected to work
	$CMD "${PIV[@]}" "${REFDIR[@]}" "${GEFDIR[@]}" 
	;;

    pgp.ef)
	echo Select AID, try and read EF.DIR, not expected to work
	$CMD "${PGP[@]}" "${REFDIR[@]}" "${GERDIR[@]}"
	;;


    piv.login.state)
	echo test if PIV login state is maintained
	$CMD "${PIV[@]}" "${V800[@]}" "${V80P[@]}" "${V800[@]}" \
	    "${PIV[@]}" "${V800[@]}" \
	    "${PGP[@]}" "${V800[@]}"\
	        "${PIV[@]}" "${V800[@]}"
      #"${MUSCLE[@]}" "${V800[@]}" 
	;;

    pgp.login.state)
	echo test if PGP login state query works
	$CMD "${PGP[@]}" "${V820[@]}" "${V82P[@]}" "${V820[@]}" \
	    "${PGP[@]}" "${V820[@]}" \
	    "${PIV[@]}" "${V820[@]}"\
	        "${PGP[@]}" "${V820[@]}"
      #"${MUSCLE[@]}" "${V820[@]}" \
	;;
    *)
	echo unknown arg $arg
	;;
  esac
  shift
done
