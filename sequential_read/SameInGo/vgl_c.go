package main

import (
	"fmt"
	"math"
	"os"
	"sort"
	"time"
)

//Source by MichaelF

const TEXTLENGTH int = 128
const TEXTLENGTH_DOUBLE int = TEXTLENGTH * 2
const TEXTLENGTH_NUMBER int = 8

type T_ADDRESS struct {
	acTown          [TEXTLENGTH]uint8
	acTownCounty    [TEXTLENGTH]uint8
	acTownCountyOld [TEXTLENGTH]uint8
	acTownDistrict  [TEXTLENGTH]uint8
	acPostcode      [6]uint8
	acStreet        [TEXTLENGTH]uint8
	acStreetNumber  [TEXTLENGTH_NUMBER]uint8
	acLattitude     [16]uint8
	acLongitude     [16]uint8
	fLattitude      float64
	fLongitude      float64
	fDistance       float64
	fAngle          float64
}

var angleErrors int = 0
var distanceErrors int = 0
var lineCount int = 0
var fileOpenActions = 0
var fEarthRadius float64 = 6371000.785

const fPI = 3.14159265358979323846

func isNaN(f float64) (is bool) {
	// IEEE 754 says that only NaNs satisfy f != f.
	return f != f
}

func Float64FromUint8(data *[16]uint8) float64 {

	var fValue float64 = 0.0
	var fFaktor float64 = 1.0
	var f int = 0
	var decimal bool = false

	for ; (f < 16) && (decimal == false); f++ {
		switch (*data)[f] {
		case '0':
			fValue = fValue * 10.0
		case '1':
			fValue = fValue*10.0 + 1.0
		case '2':
			fValue = fValue*10.0 + 2.0
		case '3':
			fValue = fValue*10.0 + 3.0
		case '4':
			fValue = fValue*10.0 + 4.0
		case '5':
			fValue = fValue*10.0 + 5.0
		case '6':
			fValue = fValue*10.0 + 6.0
		case '7':
			fValue = fValue*10.0 + 7.0
		case '8':
			fValue = fValue*10.0 + 8.0
		case '9':
			fValue = fValue*10.0 + 9.0
		case '.':
			decimal = true
		case 0:
			return fValue
		}
	}

	for ; f < 16; f++ {
		switch (*data)[f] {
		case '0':
			fValue = fValue * 10.0
		case '1':
			fValue = fValue*10.0 + 1.0
		case '2':
			fValue = fValue*10.0 + 2.0
		case '3':
			fValue = fValue*10.0 + 3.0
		case '4':
			fValue = fValue*10.0 + 4.0
		case '5':
			fValue = fValue*10.0 + 5.0
		case '6':
			fValue = fValue*10.0 + 6.0
		case '7':
			fValue = fValue*10.0 + 7.0
		case '8':
			fValue = fValue*10.0 + 8.0
		case '9':
			fValue = fValue*10.0 + 9.0
		case 0:
			return fValue * fFaktor
		}
		fFaktor = fFaktor / 10.0
	}

	return fValue * fFaktor
}

func check(e error) {
	if e != nil {
		panic(e)
	}
}

func leseDaten(u8InputBuffer []uint8, pu8InputBufferStart *uint8, u32Length int, tAddresses *[]T_ADDRESS) {

	var numberLines uint32 = 0
	var f int = 0
	var line int = 0
	var position uint = 0

	for f = 0; f < u32Length; f++ {
		if '\n' == u8InputBuffer[f] {
			numberLines++
		}
	}
	*tAddresses = make([]T_ADDRESS, numberLines, numberLines)

	for f = 0; f < u32Length; f++ {
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acTown[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acStreet[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acStreetNumber[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acPostcode[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acTownCounty[position] = u8InputBuffer[f]
				position++
			}
		}

		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acTownCountyOld[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acTownDistrict[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if ';' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acLattitude[position] = u8InputBuffer[f]
				position++
			}
		}
		f++
		position = 0
		for ; f < u32Length; f++ {
			if '\n' == u8InputBuffer[f] {
				break
			} else {
				(*tAddresses)[line].acLongitude[position] = u8InputBuffer[f]
				position++
			}
		}
		(*tAddresses)[line].fLattitude = Float64FromUint8(&(*tAddresses)[line].acLattitude)
		(*tAddresses)[line].fLongitude = Float64FromUint8(&(*tAddresses)[line].acLongitude)
		(*tAddresses)[line].fDistance = 0.0
		(*tAddresses)[line].fAngle = 0.0
		line++
	}

	//fmt.Printf("Found %v lines, converted %v\n", numberLines, line)
	lineCount = int(numberLines)
	//return tAddresses
}

func CalcDistanceToFernsehturm(linecount int, tAddresses *[]T_ADDRESS) {

	var line int = 0
	var phiCalcA float64 = 0.0
	var lambdaCalcA float64 = 0.0
	var phiCalcB float64 = 0.0
	var lambdaCalcB float64 = 0.0
	var fFernsehturmPhiB float64 = 52.520803
	var fFernsehturmLambdaB float64 = 13.40945
	var zeta float64 = 0.0
	var alpha float64 = 0.0
	var beta float64 = 0.0
	var alphaGrad float64 = 0.0
	var betaGrad float64 = 0.0

	for ; line < linecount; line++ {
		phiCalcB = fFernsehturmPhiB * fPI / 180.0
		lambdaCalcB = fFernsehturmLambdaB * fPI / 180.0

		phiCalcA = (*tAddresses)[line].fLattitude * fPI / 180.0
		lambdaCalcA = (*tAddresses)[line].fLongitude * fPI / 180.0

		zeta = math.Acos(math.Sin(phiCalcA)*math.Sin(phiCalcB) + math.Cos(phiCalcA)*math.Cos(phiCalcB)*math.Cos(lambdaCalcB-lambdaCalcA))
		//var f1 float64 = math.Sin(phiCalcA) * math.Sin(phiCalcB)
		//var f2 float64 = math.Cos(phiCalcA) * math.Cos(phiCalcB)
		//var f3 float64 = math.Cos(lambdaCalcB - lambdaCalcA)
		//var f4 float64 = f1 + f2*f3
		//zeta = math.Acos(f4)

		alpha = math.Acos((math.Sin(phiCalcB) - math.Sin(phiCalcA)*math.Cos(zeta)) / (math.Cos(phiCalcA) * math.Sin(zeta)))
		//var f5 float64 = math.Sin(phiCalcB)
		//var f6 float64 = math.Sin(phiCalcA) * math.Cos(zeta)
		//var fZAlpha float64 = f5 - f6
		//var fNAlpha float64 = math.Cos(phiCalcA) * math.Sin(zeta)
		//var f7 float64 = fZAlpha / fNAlpha
		//alpha = math.Acos(f7)
		beta = math.Acos((math.Sin(phiCalcA) - math.Sin(phiCalcB)*math.Cos(zeta)) / (math.Cos(phiCalcB) * math.Sin(zeta)))
		//var f8 float64 = math.Sin(phiCalcA)
		//var f9 float64 = math.Sin(phiCalcB) * math.Cos(zeta)
		//var fZBeta float64 = f8 - f9
		//var fNBeta float64 = math.Cos(phiCalcB) * math.Sin(zeta)
		//var f10 float64 = fZBeta / fNBeta
		//beta = math.Acos(f10)

		if lambdaCalcA <= lambdaCalcB {
			if phiCalcA >= 0 {
				alphaGrad = alpha * 180.0 / fPI
			} else {
				alphaGrad = 180.0 - alpha*180.0/fPI
			}
			if phiCalcB >= 0 {
				betaGrad = 180.0 - beta*180.0/fPI
			} else {
				betaGrad = beta * 180.0 / fPI
			}

		} else {

			if phiCalcA >= 0 {
				alphaGrad = 360.0 - alpha*180.0/fPI
			} else {
				alphaGrad = 180.0 + alpha*180.0/fPI
			}
			if phiCalcB >= 0 {
				betaGrad = 180.0 + beta*180.0/fPI
			} else {
				betaGrad = 360.0 - beta*180.0/fPI
			}
		}

		(*tAddresses)[line].fAngle = betaGrad
		(*tAddresses)[line].fAngle = math.Round(alphaGrad*1000) / 1000
		//var sA1 int = int((*tAddresses)[line].fAngle)
		//var sA01 int = int((*tAddresses)[line].fAngle*1000) % 1000
		if isNaN((*tAddresses)[line].fAngle) {
			//print("error")
			angleErrors++
		}
		(*tAddresses)[line].fDistance = math.Round(zeta*fEarthRadius*1000) / 1000
		if isNaN((*tAddresses)[line].fDistance) {
			//print("error")
			distanceErrors++
		}

	}
}

func SortByDistance(linecount int, tAddresses *[]T_ADDRESS) {
	sort.Slice((*tAddresses), func(i, j int) bool {
		if (*tAddresses)[i].fDistance < (*tAddresses)[j].fDistance {
			return true
		} else {
			if (*tAddresses)[i].fDistance == (*tAddresses)[j].fDistance {
				if (*tAddresses)[i].fAngle <= (*tAddresses)[j].fAngle {
					return true
				} else {
					return false
				}
			}
			return false
		}
		return true
	})
}

func PrepareArrayPostcode(src *[6]uint8, dst *[TEXTLENGTH_DOUBLE]uint8) {

	var sF int = 0

	for ; sF < 6; sF++ {
		(*dst)[sF] = (*src)[sF]
	}
}

func PrepareArrayStreetNb(src *[TEXTLENGTH_NUMBER]uint8, dst *[TEXTLENGTH_DOUBLE]uint8) {

	var sF int = 0

	for ; sF < TEXTLENGTH_NUMBER; sF++ {
		(*dst)[sF] = (*src)[sF]
		if 0 == (*src)[sF] {
			return
		}
	}
}

/*     0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F */
var charReplace = [256]uint8{
	0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x20, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, '\'', 0x02, 0x02, 0x02, 0x02, 0x02, '-', 0x02, 0x02,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,

	0x02, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x02, 0x02, 0x02, 0x02, 0x02,

	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 'S', 0x02, 'A', 0x02, 'Z', 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 's', 0x00, 'a', 0x02, 'z', 0x02,
	0x02, 0x02, 'c', 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	'A', 'A', 'A', 'A', 0x01, 'A', 'A', 'C', 'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
	'D', 'N', 'O', 'O', 'O', 'O', 0x01, 0x02, 'O', 'U', 'U', 'U', 0x01, 'Y', 0x02, 0x01,
	'a', 'a', 'a', 'a', 0x01, 'a', 'a', 'c', 'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
	'o', 'n', 'o', 'o', 'o', 'o', 0x01, 0x02, 'o', 'u', 'u', 'u', 0x01, 'y', 'p', 'y'}

func PrepareArray(src *[TEXTLENGTH]uint8, dst *[TEXTLENGTH_DOUBLE]uint8) {

	var sF int = 0
	var sFD int = 0

	for ; sF < TEXTLENGTH; sF++ {

		switch charReplace[src[sF]] {
		case 0:
			{
				(*dst)[sFD] = 0
				return
			}
		case 2:
			{

			}
		case 1:
			{
				switch (*src)[sF] {
				case 0xC4 /*Ä*/ :
					{
						(*dst)[sFD] = 'A'
						sFD++
						(*dst)[sFD] = 'E'
						sFD++
					}
				case 0xD6 /*Ö*/ :
					{
						(*dst)[sFD] = 'O'
						sFD++
						(*dst)[sFD] = 'E'
						sFD++
					}
				case 0xDC /*Ü*/ :
					{
						(*dst)[sFD] = 'U'
						sFD++
						(*dst)[sFD] = 'E'
						sFD++
					}
				case 0xE4 /*ä*/ :
					{
						(*dst)[sFD] = 'a'
						sFD++
						(*dst)[sFD] = 'e'
						sFD++
					}
				case 0xF6 /*ö*/ :
					{
						(*dst)[sFD] = 'o'
						sFD++
						(*dst)[sFD] = 'e'
						sFD++
					}
				case 0xFC /*ü*/ :
					{
						(*dst)[sFD] = 'u'
						sFD++
						(*dst)[sFD] = 'e'
						sFD++
					}
				case 0xDF /*ß*/ :
					{
						(*dst)[sFD] = 's'
						sFD++
						(*dst)[sFD] = 's'
						sFD++
					}

				}

			}

		default:

			{
				(*dst)[sFD] = charReplace[(*src)[sF]]
				sFD++
			}
		}
	}

}

func CompareArray2(A *[TEXTLENGTH_DOUBLE]uint8, B *[TEXTLENGTH_DOUBLE]uint8) int8 {

	var sFA int = 0
	var sFB int = 0
	var charA uint8
	var charB uint8
	var modeA uint8 = 0
	var modeB uint8 = 0

	for ; sFA < TEXTLENGTH_DOUBLE; sFA++ {

		switch modeA {
		case 0:
			{
				charA = (*A)[sFA]
				switch charReplace[charA] {
				case 0:
					{
					}
				case 1:
					{
					}
				default:
					{
					}

				}
			}
		}
		switch modeB {
		case 0:
			{
				charB = (*B)[sFB]
			}
		}

		if charA != charB {

			if charA < charB {
				return 1
			}
			if charA > charB {
				return -1
			}
			if charA != 0 && charB == 0 {
				return -1
			}
			if charA == 0 && charB != 0 {
				return 1
			}
		}

		sFB++
	}

	return 0
}

func CompareArray(A *[TEXTLENGTH_DOUBLE]uint8, B *[TEXTLENGTH_DOUBLE]uint8) int8 {

	var sF int = 0

	for ; sF < TEXTLENGTH_DOUBLE; sF++ {
		if (*A)[sF] == 0 && (*B)[sF] == 0 {
			return 0
		}

		if (*A)[sF] != (*B)[sF] {

			if (*A)[sF] < (*B)[sF] {
				return 1
			}
			if (*A)[sF] > (*B)[sF] {
				return -1
			}
			if (*A)[sF] != 0 && (*B)[sF] == 0 {
				return -1
			}
			if (*A)[sF] == 0 && (*B)[sF] != 0 {
				return 1
			}
		}
	}
	return 0
}
func CompareArray256(A *[TEXTLENGTH]uint8, B *[TEXTLENGTH]uint8) int8 {

	var sF int = 0

	for ; sF < TEXTLENGTH; sF++ {
		if (*A)[sF] == 0 && (*B)[sF] == 0 {
			return 0
		}

		if (*A)[sF] != (*B)[sF] {

			if (*A)[sF] < (*B)[sF] {
				return 1
			}
			if (*A)[sF] > (*B)[sF] {
				return -1
			}
			if (*A)[sF] != 0 && (*B)[sF] == 0 {
				return -1
			}
			if (*A)[sF] == 0 && (*B)[sF] != 0 {
				return 1
			}
		}
	}
	return 0
}

func CompareArrayFolder(A *[1024]uint8, B *[1024]uint8) int8 {

	var sF int = 0

	for ; sF < 1024; sF++ {
		if (*A)[sF] == 0 && (*B)[sF] == 0 {
			return 0
		}

		if (*A)[sF] != (*B)[sF] {

			if (*A)[sF] < (*B)[sF] {
				return 1
			}
			if (*A)[sF] > (*B)[sF] {
				return -1
			}
			if (*A)[sF] != 0 && (*B)[sF] == 0 {
				return -1
			}
			if (*A)[sF] == 0 && (*B)[sF] != 0 {
				return 1
			}
		}
	}
	return 0
}

var acCompareA [TEXTLENGTH_DOUBLE]uint8
var acCompareB [TEXTLENGTH_DOUBLE]uint8

func SortByAddress(linecount int, tAddresses *[]T_ADDRESS) {
	sort.Slice(*tAddresses, func(i, j int) bool {
		var result int8
		//var result bool = false
		// town
		PrepareArray(&(*tAddresses)[i].acTown, &acCompareA)
		PrepareArray(&(*tAddresses)[j].acTown, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		// county
		PrepareArray(&(*tAddresses)[i].acTownCounty, &acCompareA)
		PrepareArray(&(*tAddresses)[j].acTownCounty, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		// district
		PrepareArray(&(*tAddresses)[i].acTownDistrict, &acCompareA)
		PrepareArray(&(*tAddresses)[j].acTownDistrict, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		// postcode
		PrepareArrayPostcode(&(*tAddresses)[i].acPostcode, &acCompareA)
		PrepareArrayPostcode(&(*tAddresses)[j].acPostcode, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		// street
		PrepareArray(&(*tAddresses)[i].acStreet, &acCompareA)
		PrepareArray(&(*tAddresses)[j].acStreet, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		// streetnumber
		PrepareArrayStreetNb(&(*tAddresses)[i].acStreetNumber, &acCompareA)
		PrepareArrayStreetNb(&(*tAddresses)[j].acStreetNumber, &acCompareB)
		result = CompareArray(&acCompareA, &acCompareB)
		if 0 != result {
			if 1 == result {
				return true
			} else {
				return false
			}
		}

		return false
	})
}

func WriteFile(linecount int, tAddresses *[]T_ADDRESS, filename int) {

	var sF int = 0
	var sFF int = 0
	var u8Leer [100]byte
	var u8LineEnde [4]byte
	var textLen int = 0
	var textLen2 int = 0
	var sFileName string = ""
	var sA1 int
	var sA01 int

	for sF = 0; sF < 100; sF++ {
		u8Leer[sF] = ' '
	}
	u8Leer[99] = 0
	u8LineEnde[0] = 0xB0
	u8LineEnde[1] = '\n'
	u8LineEnde[2] = 0
	u8LineEnde[3] = 0

	switch filename {
	case 0:
		sFileName = "c:/tmp/output_streets.txt"
	case 1:
		sFileName = "c:/tmp/output_distances.txt"
	}

	file, err := os.OpenFile(sFileName, os.O_TRUNC|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		print("failed creating file %s\r\n", err)
	}

	for sF = 0; sF < linecount; sF++ {
		file.Write((*tAddresses)[sF].acPostcode[0:5])
		for sFF = 0; sFF < TEXTLENGTH; sFF++ {
			if 0 == (*tAddresses)[sF].acTown[sFF] {
				textLen = sFF
				sFF = TEXTLENGTH
			}
		}
		for sFF = 0; sFF < TEXTLENGTH; sFF++ {
			if 0 == (*tAddresses)[sF].acTownDistrict[sFF] {
				textLen2 = sFF
				sFF = TEXTLENGTH
			}
		}
		fmt.Fprintf(file, " %s - %s,", (*tAddresses)[sF].acTown[0:textLen], (*tAddresses)[sF].acTownDistrict[0:textLen2])
		file.Write(u8Leer[0:(75 - textLen - textLen2)])

		for sFF = 0; sFF < TEXTLENGTH; sFF++ {
			if 0 == (*tAddresses)[sF].acStreet[sFF] {
				textLen = sFF
				sFF = TEXTLENGTH
			}
		}
		for sFF = 0; sFF < TEXTLENGTH; sFF++ {
			if 0 == (*tAddresses)[sF].acStreetNumber[sFF] {
				textLen2 = sFF
				sFF = TEXTLENGTH
			}
		}
		fmt.Fprintf(file, " %s %s", (*tAddresses)[sF].acStreet[0:textLen], (*tAddresses)[sF].acStreetNumber[0:textLen2])
		file.Write(u8Leer[0:(65 - textLen - textLen2)])

		if (*tAddresses)[sF].fDistance > 1000.0 {
			var s1000 int = int((*tAddresses)[sF].fDistance) / 1000
			var s1 int = int((*tAddresses)[sF].fDistance) % 1000
			var s01 int = int((*tAddresses)[sF].fDistance*1000) % 1000
			if !isNaN((*tAddresses)[sF].fAngle) {
				sA1 = int((*tAddresses)[sF].fAngle)
				sA01 = int((*tAddresses)[sF].fAngle*1000) % 1000
			} else {
				sA1 = 0
				sA01 = 0
			}
			fmt.Fprintf(file, "=% 8d.%03d,%03dm / %3d,%03d", s1000, s1, s01, sA1, sA01)
		} else {
			var s1 int = int((*tAddresses)[sF].fDistance)
			var s01 int = int((*tAddresses)[sF].fDistance*1000) % 1000
			//var sA1 int = int((*tAddresses)[sF].fAngle)
			//var sA01 int = int((*tAddresses)[sF].fAngle*1000) % 1000
			if !isNaN((*tAddresses)[sF].fAngle) {
				sA1 = int((*tAddresses)[sF].fAngle)
				sA01 = int((*tAddresses)[sF].fAngle*1000) % 1000
			} else {
				sA1 = 0
				sA01 = 0
			}
			fmt.Fprintf(file, "=% 12d,%03dm / %3d,%03d", s1, s01, sA1, sA01)
		}
		file.Write(u8LineEnde[0:2])
	}

	file.Close()
}

func WriteToFolder(linecount int, tAddresses *[]T_ADDRESS, acFolderStart [128]uint8) {

	var sF int = 0
	var sFF int = 0
	var acFolderOld [1024]uint8
	var acFolder [1024]uint8
	var lenFolderStart int
	var lengthStreetNb int = 0
	var lengthCountyOld int = 0
	//var lengthLattitude int = 0
	//var lengthLongitude int = 0
	var openFile bool = true

	file, err := os.OpenFile("c:/tmp/dummy.txt", os.O_TRUNC|os.O_CREATE|os.O_WRONLY, 0644)
	file.Close()

	for ; sF < 128; sF++ {
		if 0 == acFolderStart[sF] {
			acFolder[sF] = '/'
			acFolderOld[sF] = '/'
			lenFolderStart = sF + 1
			acFolder[sF+1] = 0
			acFolderOld[sF+1] = 0
			break
		}
		acFolder[sF] = acFolderStart[sF]
		acFolderOld[sF] = acFolderStart[sF]
	}

	for sF = 0; sF < linecount; sF++ {

		if openFile == true {

			var sPos int = 0
			for sFF = lenFolderStart; sFF < 1024; sFF++ {

				if 0 == (*tAddresses)[sF].acTown[sPos] {
					break
				}
				acFolder[sFF] = (*tAddresses)[sF].acTown[sPos]
				sPos++
			}
			acFolder[sFF] = '/'
			sFF++
			sPos = 0
			for ; sFF < 1024; sFF++ {

				if 0 == (*tAddresses)[sF].acTownCounty[sPos] {
					break
				}
				acFolder[sFF] = (*tAddresses)[sF].acTownCounty[sPos]
				sPos++
			}
			acFolder[sFF] = '/'
			sFF++
			sPos = 0
			for ; sFF < 1024; sFF++ {

				if 0 == (*tAddresses)[sF].acTownDistrict[sPos] {
					break
				}
				acFolder[sFF] = (*tAddresses)[sF].acTownDistrict[sPos]
				sPos++
			}

			if 0 != CompareArrayFolder(&acFolderOld, &acFolder) {

				os.MkdirAll(string(acFolder[:sFF]), os.ModePerm)
			}

			acFolder[sFF] = '/'
			sFF++
			sPos = 0
			for ; sFF < 1024; sFF++ {

				if 0 == (*tAddresses)[sF].acStreet[sPos] {
					break
				}
				acFolder[sFF] = (*tAddresses)[sF].acStreet[sPos]
				sPos++
			}
			acFolder[sFF] = '.'
			sFF++
			acFolder[sFF] = 'c'
			sFF++
			acFolder[sFF] = 's'
			sFF++
			acFolder[sFF] = 'v'
			sFF++

			fileOpenActions++
			file, err = os.OpenFile(string(acFolder[:sFF]), os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
			if err != nil {
				print("failed creating file %s\r\n", err)
			}
		}

		//var length int
		for lengthStreetNb = 0; lengthStreetNb < TEXTLENGTH; lengthStreetNb++ {
			if 0 == (*tAddresses)[sF].acStreetNumber[lengthStreetNb] {
				//lengthStreetNb = length
				break
			}
		}
		for lengthCountyOld = 0; lengthCountyOld < TEXTLENGTH; lengthCountyOld++ {
			if 0 == (*tAddresses)[sF].acTownCountyOld[lengthCountyOld] {
				//lengthCountyOld = length
				break
			}
		}
		/*
			for length = 0; length < TEXTLENGTH; length++ {
				if 0 == (*tAddresses)[sF].acLattitude[length] {
					lengthLattitude = length
					break
				}
			}
			for length = 0; length < TEXTLENGTH; length++ {
				if 0 == (*tAddresses)[sF].acLongitude[length] {
					lengthLongitude = length
					break
				}
			}*/
		var s1 int = int((*tAddresses)[sF].fDistance)
		var s01 int = int((*tAddresses)[sF].fDistance*1000) % 1000
		var sA1 int = int((*tAddresses)[sF].fAngle)
		var sA01 int = int((*tAddresses)[sF].fAngle*1000) % 1000
		if !isNaN((*tAddresses)[sF].fAngle) {
			fmt.Fprintf(file, "%s;%s;%s;%f;%f;%d.%03d;%d.%03d\n", (*tAddresses)[sF].acStreetNumber[:lengthStreetNb], (*tAddresses)[sF].acPostcode[0:5], (*tAddresses)[sF].acTownCountyOld[:lengthCountyOld], (*tAddresses)[sF].fLattitude, (*tAddresses)[sF].fLongitude, s1, s01, sA1, sA01)
		} else {
			fmt.Fprintf(file, "%s;%s;%s;%f;%f;%d.%03d;0.0\n", (*tAddresses)[sF].acStreetNumber[:lengthStreetNb], (*tAddresses)[sF].acPostcode[0:5], (*tAddresses)[sF].acTownCountyOld[:lengthCountyOld], (*tAddresses)[sF].fLattitude, (*tAddresses)[sF].fLongitude, s1, s01)
		}

		if (sF+1) < linecount &&
			0 != CompareArray256(&(*tAddresses)[sF].acStreet, &(*tAddresses)[sF+1].acStreet) {
			file.Close()
			openFile = true

			for sFF = 0; sFF < 1024; sFF++ {
				acFolderOld[sFF] = acFolder[sFF]
				if 0 == acFolder[sFF] {
					break
				}
			}

		} else {
			openFile = false
		}

	}
}
func ReadCSVFile(szFolder string, szTown string, szCounty string, szDistrict string, szStreetName string, tAddresses *[]T_ADDRESS) {

	var sF int = 0
	var tAddress T_ADDRESS
	var sPos int
	var u8Buffer [16]uint8
	var szFileName string = szFolder + "/" + szTown + "/" + szCounty + "/" + szDistrict + "/" + szStreetName
	u8Town := []uint8(szTown)
	u8County := []uint8(szCounty)
	u8District := []uint8(szDistrict)
	u8StreetName := []uint8(szStreetName)

	file, err := os.OpenFile(szFileName, os.O_RDONLY, 0644)
	if err != nil {
		print("failed creating file %s\r\n", err)
	}

	fFileLength, err := file.Stat()
	check(err)

	u8InputBuffer := make([]byte, fFileLength.Size())
	n1, err := file.Read(u8InputBuffer)
	check(err)

	for ; sF < n1; sF++ {
		//1;10713;Wilmersdorf;52.482187;13.318354;7513.930;55.112
		for sPos = 0; sF < n1; sF++ {
			if ';' == u8InputBuffer[sF] {
				tAddress.acStreetNumber[sPos] = 0
				break
			}
			tAddress.acStreetNumber[sPos] = u8InputBuffer[sF]
			sPos++
		}

		sF++
		for sPos = 0; sF < n1; sF++ {
			if ';' == u8InputBuffer[sF] {
				tAddress.acPostcode[sPos] = 0
				break
			}
			tAddress.acPostcode[sPos] = u8InputBuffer[sF]
			sPos++
		}

		sF++
		for sPos = 0; sF < n1; sF++ {
			if ';' == u8InputBuffer[sF] {
				tAddress.acTownCountyOld[sPos] = 0
				break
			}
			tAddress.acTownCountyOld[sPos] = u8InputBuffer[sF]
			sPos++
		}

		sF++
		for sPos = 0; sF < n1; sF++ {
			if ';' == u8InputBuffer[sF] {
				tAddress.acLattitude[sPos] = 0
				break
			}
			tAddress.acLattitude[sPos] = u8InputBuffer[sF]
			sPos++
		}

		sF++
		for sPos = 0; sF < n1; sF++ {
			var char uint8 = u8InputBuffer[sF]
			if ';' == char {
				tAddress.acLongitude[sPos] = 0
				break
			}
			tAddress.acLongitude[sPos] = u8InputBuffer[sF]
			sPos++
		}
		sF++

		for sPos = 0; sF < n1; sF++ {
			if ';' == u8InputBuffer[sF] {
				break
			}
			u8Buffer[sPos] = u8InputBuffer[sF]
			sPos++
		}
		tAddress.fDistance = Float64FromUint8(&u8Buffer)

		for sPos = 0; sF < n1; sF++ {
			if '\n' == u8InputBuffer[sF] {
				break
			}
			u8Buffer[sPos] = u8InputBuffer[sF]
			sPos++
		}
		tAddress.fAngle = Float64FromUint8(&u8Buffer)

		var sFF int
		for sFF = 0; sFF < len(u8Town); sFF++ {
			tAddress.acTown[sFF] = u8Town[sFF]
		}
		tAddress.acTown[sFF] = 0
		for sFF = 0; sFF < len(u8County); sFF++ {
			tAddress.acTownCounty[sFF] = u8County[sFF]
		}
		tAddress.acTownCounty[sFF] = 0
		for sFF = 0; sFF < len(u8District); sFF++ {
			tAddress.acTownDistrict[sFF] = u8District[sFF]
		}
		tAddress.acTownDistrict[sFF] = 0
		for sFF = 0; sFF < len(u8StreetName); sFF++ {
			tAddress.acStreet[sFF] = u8StreetName[sFF]
		}
		tAddress.acStreet[sFF] = 0
		tAddress.fLattitude = Float64FromUint8(&tAddress.acLattitude)
		tAddress.fLongitude = Float64FromUint8(&tAddress.acLongitude)

		(*tAddresses) = append((*tAddresses), tAddress)
	}

	file.Close()
}

func ReadFilesFromFolder(linecount int, tAddresses *[]T_ADDRESS, acFolderStart [128]uint8) {

	var sF int = 0
	for ; sF < 128; sF++ {
		if 0 == acFolderStart[sF] {
			break
		}
	}

	folder := string(acFolderStart[0:sF])
	items, _ := os.ReadDir(folder)
	/* root */
	for _, item := range items {
		if item.IsDir() {
			/* town */
			//fmt.Println("Dir1: " + item.Name())
			subitemsTown, _ := os.ReadDir(folder + "/" + item.Name())
			for _, subitemTown := range subitemsTown {
				if subitemTown.IsDir() {
					/* counties */
					//fmt.Println("Dir2: " + folder + "/" + item.Name() + "/" + subitemTown.Name())
					subitemsCounty, _ := os.ReadDir(folder + "/" + item.Name() + "/" + subitemTown.Name())
					for _, subitemCounty := range subitemsCounty {

						if subitemCounty.IsDir() {
							/*district */
							//fmt.Println("Dir3: " + folder + "/" + item.Name() + "/" + subitemTown.Name() + "/" + subitemCounty.Name())
							subitemsDistrict, _ := os.ReadDir(folder + "/" + item.Name() + "/" + subitemTown.Name() + "/" + subitemCounty.Name())
							for _, subitemDistrict := range subitemsDistrict {
								if subitemDistrict.IsDir() {
									fmt.Println("Dir4: " + folder + "/" + item.Name() + "/" + subitemTown.Name() + "/" + subitemCounty.Name() + "/" + subitemDistrict.Name())
								} else {
									// handle file there
									//fmt.Println("File:" + folder + "/" + item.Name() + "/" + subitemTown.Name() + "/" + subitemCounty.Name() + "/" + subitemDistrict.Name())
									ReadCSVFile(folder, item.Name(), subitemTown.Name(), subitemCounty.Name(), subitemDistrict.Name(), tAddresses)
								}
							}
						} else {
							// handle file there
							//fmt.Println(folder + "/" + item.Name() + "/" + subitemTown.Name() + "/" + subitemCounty.Name())
							ReadCSVFile(folder, item.Name(), subitemTown.Name(), subitemCounty.Name(), " ", tAddresses)
						}

					}

					// handle file there
					//mt.Println(item.Name() + "/" + subitem.Name())
				}
			}
		} else {
			// handle file there
			fmt.Println(item.Name())
		}
	}

	(*tAddresses) = (*tAddresses)[1:]
}

func main() {
	//dat, err = os.ReadFile("c:/Projekte/go_vgl/berlin_infos.dat")
	//check(err)
	fmt.Printf("Reading file\n")
	startReadIn := time.Now()
	f, err := os.Open("D:/Projekte/govgl/berlin_infos.dat")
	check(err)
	fFileLength, err := f.Stat()
	check(err)

	u8InputBuffer := make([]byte, fFileLength.Size())
	n1, err := f.Read(u8InputBuffer)
	check(err)
	durationReadIn := time.Since(startReadIn)
	f.Close()

	fmt.Printf("Read file with %v bytes in %vms = %vus\n", n1, float64(durationReadIn.Microseconds())/1000.0, durationReadIn.Microseconds())

	startLines := time.Now()
	tAddresses := make([]T_ADDRESS, 10, 10)
	leseDaten(u8InputBuffer, &u8InputBuffer[0], n1, &tAddresses)
	durationLines := time.Since(startLines)
	fmt.Printf("Read %v lines in %vms = %vus\n", lineCount, float64(durationLines.Microseconds())/1000.0, durationLines.Microseconds())

	startFernsehTurm := time.Now()
	CalcDistanceToFernsehturm(lineCount, &tAddresses)
	durationFernsehturm := time.Since(startFernsehTurm)
	fmt.Printf("Calc Distance to Fernsehturm in %vms = %vus, error %v %v\n", float64(durationFernsehturm.Microseconds())/1000.0, durationFernsehturm.Microseconds(), angleErrors, distanceErrors)

	startSortByAddress := time.Now()
	SortByAddress(lineCount, &tAddresses)
	durationSortByAddress := time.Since(startSortByAddress)
	fmt.Printf("Sort by Address in %vs = %vms = %vus\n", float64(durationSortByAddress.Microseconds())/1000000.0, float64(durationSortByAddress.Microseconds())/1000.0, durationSortByAddress.Microseconds())

	startWriteFile := time.Now()
	WriteFile(lineCount, &tAddresses, 0)
	durationWriteFile := time.Since(startWriteFile)
	fmt.Printf("Wrote output_streets.txt in %vs = %vms = %vus\n", float64(durationWriteFile.Microseconds())/1000000.0, float64(durationWriteFile.Microseconds())/1000.0, durationWriteFile.Microseconds())

	startSortByDistance := time.Now()
	SortByDistance(lineCount, &tAddresses)
	durationSortByDistance := time.Since(startSortByDistance)
	fmt.Printf("Sort by Distance and Angle to Fernsehturm in %vms = %vus\n", float64(durationSortByDistance.Microseconds())/1000.0, durationSortByDistance.Microseconds())

	startWriteFile2 := time.Now()
	WriteFile(lineCount, &tAddresses, 1)
	durationWriteFile2 := time.Since(startWriteFile2)
	fmt.Printf("Wrote output_distances.txt in %vs = %vms = %vus\n", float64(durationWriteFile2.Microseconds())/100000.0, float64(durationWriteFile2.Microseconds())/1000.0, durationWriteFile2.Microseconds())

	print("Sorting by Address again\n")
	SortByAddress(lineCount, &tAddresses)
	startWriteFileToFolder := time.Now()
	var acStartFolder = [128]uint8{'C', ':', '/', 't', 'm', 'p'}
	WriteToFolder(lineCount, &tAddresses, acStartFolder)
	durationstartWriteFileToFolder := time.Since(startWriteFileToFolder)
	fmt.Printf("Wrote data to folder and files in %vs = %vms = %vus\n", float64(durationstartWriteFileToFolder.Microseconds())/1000000.0, float64(durationstartWriteFileToFolder.Microseconds())/1000.0, durationstartWriteFileToFolder.Microseconds())
	fmt.Printf("open actions %d\n", fileOpenActions)

	print("removing Addresses from RAM\n")
	startRemovingFromRAM := time.Now()
	tAddresses = make([]T_ADDRESS, 1, 1)
	durationRemovingFromRAM := time.Since(startRemovingFromRAM)
	fmt.Printf("Removed data from RAM in %vs = %vms = %vus\n", float64(durationRemovingFromRAM.Microseconds())/1000000.0, float64(durationRemovingFromRAM.Microseconds())/1000.0, durationRemovingFromRAM.Microseconds())

	startReadFilesFromFolder := time.Now()
	ReadFilesFromFolder(lineCount, &tAddresses, acStartFolder)
	durationReadFilesFromFolder := time.Since(startReadFilesFromFolder)
	lineCount = len(tAddresses)
	fmt.Printf("Read %v datasets from folders and files to RAM in %vs = %vms = %vus\n", lineCount, float64(durationReadFilesFromFolder.Microseconds())/1000000.0, float64(durationReadFilesFromFolder.Microseconds())/1000.0, durationReadFilesFromFolder.Microseconds())

	print("done")
}
