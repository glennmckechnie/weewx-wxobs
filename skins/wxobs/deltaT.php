<!DOCTYPE html>
<html>
<body>
<h1> Delta-T calculator</h1>

Ideal units are...<br>
Temperature in degrees C<br>
Humidity in %<br>
Pressure in hPa<br>
All other units listed will be converted to the above units then those conversions used.<br>
<p><b>There is no bounds limit on the numbers, so check for typos!</b><br>
Type the numbers directly into the appropriate fields.<br>
Use the <i>tab</i> key to move between fields<br>
The <i>up</i> or <i>down</i> arrows will increment or decrement the number</p>
<form id="delta" action="deltaT.php">
<p>Input temperature <b>(1 only!)</b>:<br><!-- 9.1 -->
<label for="degrees">&#176;C </label>
<input id="degrees" type="number" step="0.1" name="C" ><!--  48.4 -->
<label for="degrees">&#176;F </label>
<input id="degrees" type="number" step="0.1" name="F" >
</p><p>
Input Humidity:<br> <!-- 96.7 -->
<label for="humid">%</label>
<input id="humid" type="number" step="0.1" name="RH" >
</p><p>
Input pressure <b>(1 only!)</b>:<br> <!-- 1012 -->
<label for="pressure">hPa</label>
<input id="pressure" type="number" step="0.1" name="hPa" > <!-- 29.88 -->
<label for="pressure">inHg</label>
<input id="pressure" type="number" step="0.01" name="inHg"> <!-- 759.06 -->
<label for="pressure">mmHg</label>
<input id="pressure" type="number" step="0.01" name="mmHg" >
</p>
<input type="submit" value="Calculate delta-T">
</form>

</body>
</html>
<?php
/**
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
* details.
*/

function CtoF($x)
{
    global $F;
    $F = abs(round($x * 1.8 + 32.0, 1));
    return $F;
}
function FtoC($x)
{
    global $C;
    $C = abs(round((($x - 32.0) * 5.0 / 9.0), 1));
    return $C;
}

function inHgtohPa($x)
{
    global $hPa;
    $hPa=abs(round($x*33.86389, 2));
    return $hPa;
}

function mmHgtohPa($x)
{
    global $hPa;
    $hPa = abs(round($x/0.75006168, 2));
    return $hPa;
}

function hPatommHg($x)
{
    global $mmHg;
    $mmHg = abs(round($x*0.75006168, 2));
    return $mmHg;
}

function hPatoinHg($x)
{
    global $inHg;
    $inHg = abs(round(($x* 0.0295299875), 2));
    return $inHg;
}


function deltaT($Tc, $RH, $P)
{
    /*
    * DeltaT calculations
    * sourced from wdSearchX3.py, part of the weewx-wd package by oz greg
    * available at https://bitbucket.org/ozgreg/weewx-wd/wiki/Home
    * $Tc = outTemp.degree_C
    * $RH = outHumidity
    * $P = pressure.hPa
    * for the same formula as here, listed under 'Wet Bulb Temperature' see
    * https://www.aprweather.com/pages/calc.htm
    * and for related formula
    * http://www.bom.gov.au/climate/averages/climatology/relhum/calc-rh.pdf
    * then finally,
    * http://www.weather.gov/epz/wxcalc_rh
    * with a source description at
    * http://www.weather.gov/media/epz/wxcalc/wetBulbTdFromRh.pdf
    */
    global $dT;
    global $WBc;

    $Tdc = (($Tc - (14.55 + 0.114 * $Tc) * (1 - (0.01 * $RH)) -
        ((2.5 + 0.007 * $Tc) * (1 - (0.01 * $RH))) ** 3 -
        (15.9 + 0.117 * $Tc) * (1 - (0.01 * $RH)) ** 14));

    $E = (6.11 * 10 ** (7.5 * $Tdc / (237.7 + $Tdc)));

    $WBc = (((0.00066 * $P) * $Tc) + ((4098 * $E) / (($Tdc + 237.7) ** 2)
        * $Tdc)) / ((0.00066 * $P) + (4098 * $E) / (($Tdc + 237.7) ** 2));

    // $WetBulb = "%.1f" % $WBc + $unit.label.outTemp
    // $WetBulb = "%.1f" % ($WBc*9/5+32) + $unit.label.outTemp // degrees F
    //echo "Tdc = $Tdc E = $E WBc = $WBc :: T=$Tc : RH=$RH : P=$P<br>";
    //$DeltaT = $Tc - $WBc;
    //$dT = $Tc - $WBc;
    //echo "raw delta is $dT";
    $dT = abs(round($Tc - $WBc, 2));
    $WBc = abs(round($WBc, 3));
}

//$Tc = outTemp.degree_C
//$RH = outHumidity
//$P = pressure.hPa

$C =  $_REQUEST['C'];
$F =  $_REQUEST['F'];
$RH =  $_REQUEST['RH'];
$hPa =  $_REQUEST['hPa'];
$inHg =  $_REQUEST['inHg'];
$mmHg =  $_REQUEST['mmHg'];
if ($C || $F) {
    //echo "(C = $C || F= $F)";
    if ($C) {
        $Tc = $C;
        CtoF($C);
        echo "$C &#176C remains $Tc &#176;C : ( $F &#176;F )<br>";
    } elseif ($F) {
        FtoC($F);
        $Tc = $C;
        CtoF($C);
        //$Tc=abs(round((($F-32)*5/9), 1)); // assuming degF - return degC
        echo "$F &#176F to $Tc &#176;C : ( $C &#176;C ) <br>";
    }
}
if ($RH) {
    echo "$RH % remains $RH %<br>";
}

if ($hPa || $inHg || $mmHg) {
    //echo " (hPa = $hPa || inHg =$inHg || mmHg = $mmHg) ";
    if ($hPa) {
        $P = $hPa;
        hPatoinHg($hPa);
        hPatommHg($hPa);
        echo "$hPa hPa to $P hPa : (inHg - $inHg) : (mmHg - $mmHg) <br>";
    } elseif ($inHg) {
        //$P=abs(round($inHg*33.86389, 1)); //assuming inHg - return hPa
        inHgtohPa($inHg);
        $P=$hPa;
        hPatommHg($hPa);
        echo "$inHg inHg to $P hPa : (inHg - $inHg) : (mmHg - $mmHg)<br>";
    } elseif ($mmHg) {
        //$P=abs(round($mmHg/0.75006168, 1)); //assuming mmHg - return hPa
        mmHgtohPa($mmHg);
        $P=$hPa;
        hPatoinHg($hPa);
        echo "$mmHg mmHg to $P hPa : (inHg - $inHg) : (mmHg - $mmHg)<br>";
    }
}

if ($Tc && $RH && $P) {
    echo "($Tc && $RH && $P)";
    deltaT($Tc, $RH, $P);
    echo "<p>Using the values of...<br>Temperature of $Tc<br>
         Humidity of $RH<br>Pressure of $P</p>";
    echo "<p><b>delta-T is $dT &#176;C</b></p>";
    echo "<p><b>Wet Bulb is $WBc &#176;C</b> ($WBc +$dT ~= $Tc)</p>";
} else {
    echo "<p><b>Input values for all fields to Calculate delta-T correctly</b></p>";
    echo "<p>The values ...<br>Temperature of ' $Tc'<br>
         Humidity of ' $RH'<br>Pressure of ' $P'</p>";
}
echo "<hr><h3>From the Australian, Bureau of Meteorology (BOM)</h3><p>
     <a href=\"http://www.bom.gov.au/lam/deltat.shtml\">What is delta-T</a>
     and their
     <a href=\"http://www.bom.gov.au/info/leaflets/Pesticide-Spraying.pdf\">
     Leaflet for Pesticide Spraying (pdf)</a></p>";
echo "<hr><p>This DeltaT calculator uses formulas originally sourced from the
     Wet bulb section in  wdSearchX3.py, which is part of the weewx-wd package 
     by oz greg and  which is available at 
     <a href=\"https://bitbucket.org/ozgreg/weewx-wd/wiki/Home\"\>
     https://bitbucket.org/ozgreg/weewx-wd/wiki/Home</a>
     The Delta_T addition started from user Powerin in a weewx-user posting.</p>
     <p>That same formula is listed on <b>Andrew Revering's List of Meteorological
     Formulas</b> page, under the section 
     <a href=\"https://www.aprweather.com/pages/calc.htm\">
     Wet Bulb Temperature</a></p>
     This implementation (with calculator and conversions) is provided as
     part of the <a href=\"https://github.com/glennmckechnie/weewx-wxobs\">
     weewx-wxobs package at github</a></p>";
echo "<hr>Do not make important decisions based on this calculator, it is
     an aid only.  It is NOT authoritative.";
