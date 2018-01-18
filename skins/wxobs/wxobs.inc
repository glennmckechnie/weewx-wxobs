## Copyright 2009-2017 Tom Keffer, Matthew Wall
## Distributed under terms of GPLv3.  See LICENSE.txt for your rights.
#errorCatcher Echo
## Specifying an encoding of UTF-8 is usually safe, but if the text is actually
## in Latin-1, then you should replace the string "UTF-8" with "latin-1".  If
## you do this, you should also change the charset in 'Content-Type' as well.
#encoding UTF-8
## php inclusion to create wxobs.php.tmpl by glenn.mckechnie@gmail.com
## available at https://github.com/glennmckechnie/weewx-wxobs
## 06-09-2017 initial releas @0.01
## 08-09-2017 convert to installable skin
## 08-09-2017 add unit check and conversion for deltaT
## 13-09-2017 move sensitive info to an include file in /tmp, rather than have it
##            embedded within index.php. Don't show links.inc in index.php as it
##            breaks the flow and is redundant there. center all but debug info.
## 18-09-2017 Add aus_rain where the 24 hour rainfall count starts at $start_label a.m.
## 24-09-2017 Add functions to assist in converting the database units to those
##            chosen by the user. aus_rain becomes shift_rain, now configurable for
##            other areas
## 30-09-2017 Add checkdate and logic to validate entered dates.
## Version 0.05

   <!-- wxobs version $wxobs_version -->
   <!-- This weewx extension is available at github: https://github.com/glennmckechnie/weewx-wxobs -->
   <div class="wxobs">
      Use the following to select a date for archived records
      <!-- insert the javascript date picker -->
      <form name="form" method="POST" action="index.php">
      Choose the date:
      <input id="start_dt" name="datepicker" class="datepicker">
      <input type="submit" name="submitDate" value="Show records">
      </form>
   </div>
<?php
ini_set('display_errors', 1);
error_reporting(~0);
$start = microtime(true);

if ((@include '$include_file') === false) {
    echo "<div class=\"alert\">";
    echo "<p>The database config file has gone missing!<br>";
    echo "&nbsp;&nbsp;&nbsp;That shouldn't happen.</p>";
    echo "We are using the php default for includes of /usr/share/php<br>";
    echo "Check that the directory /usr/share/php exists, if not create it.<br>";
    echo "The file should have been accessable as<br> <i>$include_file</i><br>";
    echo "It will be regenerated on weewx's next report run,";
    echo " or when report_timing allows.<br>";
    echo "If the problem persists, consider changing";
    echo " its location as ";
    echo "wxobs/skin.conf will accept<br>";
    echo "<i>include_file = /new_path/name</i><br>";
    echo "where the path and name can be anything";
    echo " allowable by php.ini and preferably secure.<br>";
    echo "That means keeping it away from the webserver, if possible.<br>";
    echo "If all else fails, report the problem as an issue";
    echo " - to its github site.</p></div>";
}
require '$include_file';

/**
* ToDo ??
* stackoverflow.com/questions/9866139/database-locked-while-trying-to-access-from-php-script
*  $db->exec('PRAGMA journal_mode = wal;');
*
* Debugging:
* 1 and above is low level, variables, some logic.
* 3 only for delta-T final values (low level - if enabled)
* 4 only for delta-T unit conversion calcs (verbose) - if enabled
* 5 only for ordinalCompass conversion calcs (N, NE...CALM) (verbose)
*/
//temperature conversions
// /usr/share/weewx/weewx/units.py
function CtoF($x)
{
    return $x * 1.8 + 32.0;
}

function FtoC($x)
{
    return ($x - 32.0) * 5.0 / 9.0;
}

function NTC($x)
{
    return $x;
}
//pressure conversions
// $dec_plc is decimal places to display
function inHgtomBar($x, $press_plc)
{
    $x = $x * 33.863881588;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function inHgtohPa($x, $press_plc)
{
    $x = $x * 33.863881588;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function mBartohPa($x, $press_plc)
{
    $x = $x;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function mBartoinHg($x, $press_plc)
{
    $x = $x * 0.0295299875;
    $dec_plc = 3;
    return array($x, $dec_plc);
}

function NPC($x, $press_plc)
{
    $x = $x;
    $dec_plc = $press_plc;
    return array($x, $dec_plc);
}
//speed conversions
// $dec_plc is decimal places to display
function MphtoKph($x, $wind_plc)
{
    $x = $x * 1.609344;
    $dec_plc = 0;
    return array($x, $dec_plc);
}

function MphtoMps($x, $wind_plc)
{
    $x = $x * 0.44704 ;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function KphtoMph($x, $wind_plc)
{
    $x = $x * 0.621371  ;
    $dec_plc = 0;
    return array($x, $dec_plc);
}

function KphtoMps($x, $wind_plc)
{
    $x = $x * 0.277777778;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function MpstoMph($x, $wind_plc)
{
    $x = $x * 2.23693629;
    $dec_plc = 0;
    return array($x, $dec_plc);
}

function MpstoKmh($x, $wind_plc)
{
    $x = $x * 3.6;
    $dec_plc = 0;
    return array($x, $dec_plc);
}

function NSC($x, $wind_plc)
{
    $x;
    $dec_plc = $wind_plc;
    return array($x, $dec_plc);
}
// rain conversions
// $dec_plc is decimal places to display
function IntoMm($x, $rain_plc)
{
    $x = $x * 25.4;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function IntoCm($x, $rain_plc)
{
    $x = $x * 2.54;
    $dec_plc = 2;
    return array($x, $dec_plc);
}

function CmtoIn($x, $dec_plc)
{
    $x = $x * 0.3937000787 ;
    $dec_plc = 2;
    return array($x, $dec_plc);
}

function CmtoMm($x, $rain_plc)
{
    $x = $x * 10;
    $dec_plc = 1;
    return array($x, $dec_plc);
}

function MmtoIn($x, $rain_plc)
{
    $x = $x * 0.0393700787;
    $dec_plc = 2;
    return array($x, $dec_plc);
}

function MmtoCm($x, $rain_plc)
{
    $x = $x * 0.10;
    $dec_plc = 2;
    return array($x, $dec_plc);
}

function NDC($x, $rain_plc)
{
    $x =$x;
    $dec_plc = $rain_plc;
    return array($x, $dec_plc);
}

function hDate($d)
{
        //$h_read = date('d-M-Y H:i', $d);
        $h_read = date('d-M H:i', $d);
        return $h_read;
}

function shDate($d)
{
        $h_read = date('d-M', $d);
        return $h_read;
}


function ordinalCompass($degr, $debug)
{
    /*
    * Templated from weewx code. Mistakes are mine!
    * should be using windSpeed == 0, but this works too !?
    */
    global $compass;
    $default_ordinate_names = array ('N', 'NNE','NE', 'ENE', 'E', 'ESE', 'SE', 'SSE',
                          'S', 'SSW','SW', 'WSW', 'W', 'WNW', 'NW', 'NNW',
                            'N/A'); //17 objects
    if ($debug == '5') {
        echo "<p class=\"debug\">degr passed as '$degr'<br>";
    }
    // has weewx returned null wind? (zero_wind sets windDir as NULL)
    if (is_null($degr)) {
        //$compass = "'$degr' :: CALM";
        $compass = "CALM";
        if ($debug == '5') {
            echo "so degree is '$degr': compass is $compass</p>";
        }
        return $compass;
    }

    $sector_size = 360/16;
    $degree = ($degr + $sector_size/2.0) % 360.0;
    $sector = intval($degree / $sector_size);
    $compass = $default_ordinate_names[$sector];
    if ($debug == '5') {
        echo "so degree is $degree, sector is $sector: results in compass $compass</p>";
    }
}

function deltaT($Tc, $RH, $P, $debug)
{
    /*
    * A calculator deltaT.php is distributed with this script, in the master.zip
    * file or at its github location
    *
    * DeltaT calculations
    * sourced from wdSearchX3.py, part of the weewx-wd package by oz greg
    * available at https://bitbucket.org/ozgreg/weewx-wd/wiki/Home
    * and various other sites - deltaT.php mentioned above.
    * $Tc = outTemp.degree_C
    * $RH = outHumidity
    * $P = pressure.hPa
    */
    global $dT;
    if ('$unit.label.outTemp' === '&#176;F') {
         $Tc=($Tc-32)*5/9; // assuming degF - return degC
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed degF: Tc has become $Tc</p>";
        }
    } else {
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed degC: Tc remains $Tc</p>";
        }
    }
    if ('$unit.label.barometer' === ' hPa') { //leading space is critical!
        $P=$P;
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assuming hPa: P remains $P</p>";
        }
    } elseif ('$unit.label.barometer' === ' inHg') { //leading space is critical!
        $P=($P*33.86389); //assuming inHg - return hPa
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed inHg: P has become $P</p>";
        }
    } elseif ('$unit.label.barometer' === ' mmHg') { //leading space is critical!
        $P=($P/0.75006168); //assuming mmHg - return hPa
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br> assumed mmHg: P has become $P</p>";
        }
    } else {
        //echo "Unrecognized pressure units - conversion code is not present";
        $dT="?units?";
        if ($debug=='4') {
            echo "<p class=\"debug\">delta-T<br>($P) returned, assuming nothing".
                 ": P has become $P</p>";
        }
        return;
    }

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
    $dT = abs(round($Tc - $WBc, 1));
}


/*
* Note; double quoted variables are values that will be passed from the SLE,
* via cheetahGenerator. They are required to be in place for the substitution
* to work with php.
*/

$debug='$wxobs_debug';
$tr_bgrnd="";
//global $label_plc;

// Attempt to get some meaningful table headings, dependent on xxx_interval options
if ('$arch_interval' === '$ext_interval') {
    $tflag="Total";
    $avflag="(Average)";
    $iflag=" (averaged over each interval)";
} else {
    $tflag="";
    $avflag="";
    $iflag=" (as single readings)";
}

// then define a bunch of long strings for use in <th> @ 3 locations

if ($php_dbase === 'mysql') {
    $st_date = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
              "Date (in day-month-year format) and Time.\">".
              "Date&nbsp;&nbsp;&nbsp;&nbsp; Time</a></th>";
} elseif ($php_dbase === 'sqlite') {
    $st_date = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
              "Date (in year-month-day format) and Time.\">".
              "Date&nbsp;&nbsp;&nbsp;&nbsp; Time</a></th>";
}
$st_temp ="<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
          "Outside temperature\">Out Temp<br>$unit.label.outTemp</a></th>";
$st_dew = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
          "Temperature where water droplets can condense and dew forms.\">".
          "Dew Point<br>$unit.label.dewpoint</a></th>";
$st_rh = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
         "Relative Humidity\">Humidity<br>$unit.label.outHumidity</a></th>";
$st_wind = "<th class=\"wxheadr\" colspan=\"4\"><a title=\"".
           "Wind stats as averaged over each period.\">Wind $avflag </a></th>";
$st_baro = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
           "Barometric pressure\">Pressure<br>$unit.label.barometer</a></th>";
if ($shift_rain) {
    $st_rain = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
               "Total rainfall for each interval\nstarting from the".
               " nominated\ntime. ($start_label a.m.)\">";
} else {
    $st_rain = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"".
               " Total rainfall for each\ninterval starting from the\n".
               " nominated time.\">";
}
$st_comp = "<th class=\"wxheadr\"><a title=\"Direction the wind comes from as\n".
           " cardinal (compass) values\">Direction<br>compass</a></th>";
$st_spd = "<th class=\"wxheadr\">Speed<br>$unit.label.wind</th>";
$st_gust = "<th class=\"wxheadr\">Gust<br>$unit.label.wind</th>";
$st_degr = "<th class=\"wxheadr\"><a title=\"Direction the wind comes from in ".
           "degrees\">Direction<br>$unit.label.windDir</a></th>";

if ($php_dbase === 'mysql') {
    if ($debug>='1') {
        echo "<p class=\"debug\">database is $dbase with the name: $mysql_base</p>";
    }
    $db = new mysqli(
        $php_mysql_host,
        $php_mysql_user,
        $php_mysql_pass,
        $php_mysql_base
    );
    if ($db->connect_errno) {
        echo ("<div class=\"alert\"><p>Connection to mysql database failed:<br>
            Has it been configured correctly - these were the values sent<br>
            host=$php_mysql_host,user=$php_mysql_user,
            pass=$php_mysql_pass,base=$php_mysql_base<br>
     $db->connect_error</p></div>");
        return;
    }
} elseif ($php_dbase === 'sqlite') {
    if ($debug>='1') {
        echo "<p class=\"debug\">database is $dbase with path/name: $sqlite_db</p>";
    }
    $db = new SQLite3($php_sqlite_db);
} else {
    echo "<div class =\"alert\"><p>Failed to access the database by any known methods.</p></div>";
}


if ('$app_temp' === 'appTemp') {
    $appLabel = "<a title=\"Apparent Temperature\">App Temp";
    $app_label = "$unit.label.appTemp</a>";
} elseif ('$app_temp' === 'windchill') {
    $appLabel = 'Wind Chill';
    $app_label = '$unit.label.windchill';
} else {
    $appLabel = '$app_temp';
    $app_label = '$unit.label.outTemp';
}

if ($debug>='1') {
    echo "<p class=\"debug\">ext_interval is $ext_interval <br>".
         "arch_interval is $arch_interval<br> appTemp is defined".
         "as $app_temp with $app_label used as the units.</p>";
}
if ($debug>='1') {
    echo "<p class=\"debug\">want delta is $want_delta with show warning".
         "of $show_warning<br>".
         "('$targ_unit' === 'US' || '$unit.label.outTemp' !== '&#176;C' ||".
         "'$unit.label.barometer' !== ' hPa')<br>".
         "then, if one of the above is (true) show the warning &lt;div&gt;</p>";
}

if ($show_warning) {
    if ('$targ_unit' === 'US' || '$unit.label.outTemp' !== '&#176;C'
        || '$unit.label.barometer' !== ' hPa'
    ) {
        echo "<div class=\"alert\">
              <h2>delta-T calculation: configuration notes</h2>
              <p>Weewx reports that the database is in $targ_unit units, and /
               or that the temperature, pressure units differ from the &#176;C
              or hPa required to calculate the delta-T values correctly. This will be
              compensated for but we need to be sure that what's being reported is
              correct.</p>
              <p>Review your wxobs/skin.conf file and match the group_fields with
              your database.<br> You can also use the [[PHPUnits]] section to convert
              the database values to something else, but you will still need to
              correctly describe those new values using the group_fields.
              <pre>
              [Units]
                  [[Groups]]
                      group_temperature
                      group_pressure
              </pre>
              Again, whatever units are used in the database need to be converted
              or noted in the skin.conf file for the deltaT calculation to be
              performed correctly</p>
              <p>ie:-<br>
              If you know the database stores the temperature in &#176;C then you
              need to specify it as such. <br>
              group_temperature  = degree_C<br> If it's in &#176;F then you have the
              option to convert it using [[PHPUnits]] and then selecting the
              appropriate label (which can only be &#176;C), or leave it as &#176;F
              and let the DeltaT calcs make any adjustments within its calculations.
              <br>This allows your display to correctly show the units of your choice
              (as per your group_field selection!) and it will remain as &#176;F
              in the reports display and the deltaT calculator knows that it needs
              to convert the units to get the correct result.</p>
              <p>Likewise, If you know the database stores the pressure in hPa
              then you need to specify that. If it's inHg, mmHG or other then decide
              which you want and work through the above logic again depending on
              what you want displayed.<br>
              <p>The code will compensate correctly for &#176;C, &#176;F, hPa, inHg,
              mmHg.  BUT only if you tell it in the skin.conf file (or blind luck
              favours you!)<br>
              (Others units aren't accounted for, they'll need to be added to the
              code.)</p>
              <p>&nbsp;&nbsp;&nbsp; Then: <b>Check the results!</b></p>
              <p>Finally, when all is working to your satisfaction, you can turn
              this message off manually by uncommenting the line<br>
              <b>#show_warning = False</b><br>
              in that same config file.<br>
              (The message will also disappear if the correct units (&#176;C and hPa)
              are specified in the group_fields section.)
              </p></div>";
    }
}
// For report display purposes, prime the decimal place values as if no
// conversions are to be made.
if ('$unit.label.barometer' === ' hPa') { //leading space is critical!
    $press_plc = 1;
} elseif ('$unit.label.barometer' === ' inHg') { //leading space is critical!
    $press_plc = 3;
} elseif ('$unit.label.barometer' === ' mmHg') { //leading space is critical!
    $press_plc = 1;
} else {
    $press_plc = 0;
}

if ('$unit.label.rain' === ' mm') { //leading space is critical!
    $rain_plc = 1;
} elseif ('$unit.label.rain' === ' cm') { //leading space is critical!
    $rain_plc = 2;
} elseif ('$unit.label.rain' === ' in') { //leading space is critical!
    $rain_plc = 2;
} else {
    $rain_plc = 0;
}

if ('$unit.label.wind' === ' kph') { //leading space is critical!
    $wind_plc = 0;
} elseif ('$unit.label.wind' === ' mph') { //leading space is critical!
    $wind_plc = 0;
} elseif ('$unit.label.wind' === ' m/s') { //leading space is critical!
    $wind_plc = 1;
} else {
    $wind_plc = 0;
}

// Start of Latest observations
// This always the current date and its data only.
// Show the latest record in the database and the rainfall total for the day.
    $day_start = mktime('0', '0', '0', date('n'), date('j'), date('Y'));
    $time_now = time();
if ($debug>='1') {
    echo "<p>Latest Observations:<br>day_start = ", hDate($day_start),
     "($day_start)<br> time_now uses the time_now value ", hDate($time_now),
     " ( $time_now )</p>";
}

    $last_entry = $db->query(
        "SELECT dateTime FROM archive ORDER BY datetime DESC LIMIT 1"
    );

    if ($php_dbase === 'mysql') {
        $last_entry = $last_entry->fetch_array();
    } else {
        $last_entry = $last_entry->fetchArray();
    }
      $latest_date = $last_entry[0];

    if ($shift_rain) {
        // Calculate start day for rain calcs - some recordings start at $start_label a.m.
        $today_nine = $day_start + $rainday_start; // $start_label am today
        $past_nine = $day_start - (86400 - $rainday_start); // $start_label am yesterday
        if ($time_now <= $today_nine) {
            $rain_start = $past_nine;
            $rain_end = $rain_start + 86400 + $arch_interval;
            $t_span = "(since $start_label a.m.)";
            $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                       "Previous: Running Total&#10;starting ".
                       hDate($rain_start). "\">";
            if ($debug >= 1) {
                echo "Latest: time_now=".hDate($time_now)." : ".hDate($day_start).
                 " ($day_start day_start < today_nine $today_nine) ".
                 hDate($today_nine)." : rain_start = ".hDate($rain_start).
                 " : rain_end ".hDate($rain_end)."<br>";
            }
        } elseif ($time_now > $today_nine) {
            $rain_start =$today_nine;
            $rain_end = $rain_start + 86400 + $arch_interval;
            $t_span = "(since $start_label a.m.)";
            $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                       "Current: Running Total&#10;starting ".
                       hDate($rain_start). "\">";
            if ($debug >= 1) {
                echo "Latest: time_now=".hDate($time_now)." : ".hDate($day_start).
                 " ($day_start day_start >= today_nine $today_nine) ".
                 hDate($today_nine)." : rain_start = ".hDate($rain_start).
                 " : rain_end ".hDate($rain_end)."<br>";
            }
        }
    } else {
        $rain_start = $day_start;
        $rain_end = $time_now;
        $t_span = "(since 12 a.m.)";
        $td_rain = "<td title=\"24 hour rain starting at&#10;midnight on: ".
                   sDate($rain_start). "\">";
        if ($debug >= 1) {
            echo "Latest: 24 hours ".hDate($day_start)." ($day_start day_start
                 & rain_end $rain_end) ". hDate($rain_end)." : rain_start = "
                 .hDate($rain_start)."<br>";
        }
    }

    if ($php_dbase === 'mysql') {
        $last_values = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'), outTemp,
            $app_temp, dewPoint, outHumidity, windSpeed, windGust, windDir,
            barometer FROM archive WHERE dateTime = $latest_date"
        );
        $rain_total = $db->query(
            "SELECT sum(rain) FROM archive WHERE dateTime
            >= $rain_start and dateTime < $rain_end;"
        );
        $latest_row = $last_values->fetch_array();
        $rain_row = $rain_total->fetch_array();
    } else {
        $last_values = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
            outTemp, $app_temp, dewPoint, outHumidity, windSpeed, windGust,
            windDir, barometer FROM archive WHERE dateTime = $latest_date"
        );
        $rain_total = $db->query(
            "SELECT sum(rain) FROM archive WHERE dateTime
            >= $rain_start and dateTime < $rain_end;"
        );
        $latest_row = $last_values->fetchArray();
        $rain_row = $rain_total->fetchArray();
    }

    if ($debug>='1') {
        echo "latest date = ", hDate($latest_date), " : day_start = ",
             hDate($day_start), " : time_now = ", hDate($time_now), "<br>";
        echo "latest rain_start = ", hDate($rain_start), " : ($rain_start)".
             "rain_end = ", hDate($rain_end), "($rain_end)";
    }

    // Finish Latest grunt work
    //////////////////////////
    // Build Latest Obs html

    if ($want_delta) {
        $delta_label = "<th class=\"wxheadr\" rowspan=\"2\"><a title=\"Used as an".
                       " indicator for agricultural spraying conditions, the ideal".
                       " conditions are between the values of 2 &amp; 8\">".
                       "delta&dash;T<br>&#176;C</a></th>";
        if ($debug>='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so label is...<br>
                 $delta_label";
        }
    } else {
        $delta_label="";
        if ($debug>='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so label is...<br>
                 '$delta_label'";
        }
    }


    /*    if (date_default_timezone_get()) {
    *       echo 'date_default_timezone_set: ' . date_default_timezone_get() . '<br />';
    *       $systemTimeZone = system('date +%Z');
    *   echo "System TimeZone is $systemTimeZone <br>";
    *   }
    */
    //Create Latest table headers - ugly layout but ok in html source
    echo "<div class=\"wxdata\">\n<!-- start data tables -->\n
 <H2><A Title=\"Todays latest weather observations, at the displayed time.\">Latest Observations</a></h2>
<table class=\"wxdata\">
  <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh
   $delta_label
   $st_wind \n   $st_baro
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    if ($debug >= 1) {
        $st_day = hDate($latest_date);
        $st_day = "$st_day($latest_date)";
    } else {
        $st_day = "";
    }


    $dbase_date = $latest_row[0];
    echo "<tr><td> $dbase_date<!-- $st_day --></td>";
    $x = $tempConvert ($latest_row[1]);
    $metTemp = round($x, 1);
    echo "<td>$metTemp</td>";
    $x = $tempConvert ($latest_row[2]);
    $metappTemp = round($x, 1);
    echo "<td>$metappTemp</td>";
    $x = $tempConvert ($latest_row[3]);
    $metdew = round($x, 1);
    echo "<td>$metdew</td>";
    $humid = round($latest_row[4], 0);
    echo "<td>$humid</td>";
    if ($want_delta) {
        deltaT($latest_row[1], $latest_row[4], $latest_row[8], $debug);
        echo "<td>$dT</td>";
        if ($debug=='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
            echo "delta_data is '$dT'</p>";
        }
    } else {
        if ($debug=='3') {
            echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
            echo "delta_data is 'null'</p>";
        }
    }
    ordinalCompass($latest_row[7], $debug);
    echo "<td>$compass</td>"; //wind direction ordinal
    list($x, $dec_plc) = $speedConvert ($latest_row[5], $wind_plc);
    $metWind = round($x, $dec_plc);
    echo "<td>$metWind</td>";
    list($x, $dec_plc) = $speedConvert ($latest_row[6], $wind_plc);
    $metGust = round($x, $dec_plc);
    echo "<td>$metGust</td>";
    $windDir = round($latest_row[7], 0);
    echo "<td>$windDir</td>"; //wind direction degrees
    list($x, $dec_plc) = $pressConvert ($latest_row[8], $press_plc);
    $metPress = round($x, $dec_plc);
    echo "<td>$metPress</td>";
    list($x, $dec_plc) = $rainConvert ($rain_row[0], $rain_plc);
    $metRain = round($x, $dec_plc);
    echo "$td_rain $metRain</td></tr>";
    echo "</tbody></table>\n<!-- end latest observations -->\n<br><hr>";

    // Finished Latest Obs html
    /////////////////////////////
    // Start Min Max summaries
    // Shows the min and max values, averages and rainfall total
    //
    // This value can be passed via the URL so we'll validate the datepickers
    // return value and make sure it IS a date - also check for leap year, feb 30
    // and other nonsense.
    //var_dump(!empty($_REQUEST['datepicker']));
    if (!empty($_REQUEST['datepicker'])) {
        $m_numb = '';
        list ($dd, $m_nth, $yy)  = explode("-", $_REQUEST['datepicker']);
        //echo "<br>STARTing as m_nth = $m_nth : m_numb = $m_numb<br>";
        //var_dump((strlen($m_nth))==3);
        // should be in short month format
        if ((strlen($m_nth))==3) {
            for ($i=1; $i<=12; $i++) {
                //var_dump((strtolower(date("M", mktime(0, 0, 0, $i, 1, 0))) == strtolower($m_nth)));
                if (strtolower(date("M", mktime(0, 0, 0, $i, 1, 0))) == strtolower($m_nth)) {
                    $m_numb = $i;
                    //echo "<br>m_nth = $m_nth : m_numb = $m_numb = $i<br>";
                    break;
                }
            }
        } else {
            // but it could also be a padded number
            $m_numb = intval($m_nth);
        }
        //echo " (checkdate($m_numb, $dd, $yy))<br>";
        // Now we make sure it is a valid date but checkdate works with integer
        // inputs so we'll convert the input to integers first and store the values for
        // later use. This removes any garbage strings from injection attempts.
        // https://bugs.php.net/bug.php?id=48906
        //$yy = (checkdate(11, 16, 2017' aND BeNChMaRK(2999999,Md5(NoW())) AnD '1))
        //echo "<br><br>Valid:  yy = (checkdate($m_numb, $dd, $yy))<br>";
        $m_numb = intval($m_numb);
        $dd = intval($dd);
        $yy = intval($yy);
        //echo "<br><br>Valid & intval:  yy = (checkdate($m_numb, $dd, $yy))<br>";
        if (checkdate($m_numb, $dd, $yy)) {
            // Datepicker returned a valid date, we'll now use the sanitized inputs
            // to reconstruct the date. We will ignore the datepicker input in case an
            // injection attempt was made
            //$pick_start =  strtotime($_REQUEST['datepicker']);
            $pick_start = mktime('0', '0', '0', $m_numb, $dd, $yy);
            if ($debug>='1') {
                echo "<p class=\"debug\">Min (Max)<br> Datepicker returned ",
                     hDate($pick_start), " ($pick_start) </p>";
                echo "Datepicker returned a valid date, we'll use it!<br>";
            }
        } else {
            // Invalid date format from datepicker or URL bar. We know that
            // $day_start is good (it's ours!) so we'll trust that and use it
            // instead.
            $pick_start = $day_start;
            echo "<div class=\"info\">Datepicker didn't return a valid date,".
                 " falling back to todays date.<br></div>";
            if ($debug>='1') {
                echo "<p class=\"debug\">Min (Max)<br> Datepicker return is invalid,
                     day_start ", hDate($day_start), "( $day_start )
                     has been used</p>";
            }
        }
    } else {
        // we have nothing from datepicker so we assume today is wanted.
        // We'll use our $day_start to display a report for the current date.
        $pick_start = $day_start;
        echo "<div class=\"info\">Using todays date.<br></div>";
        if ($debug>='1') {
            echo "<p class=\"debug\">Min (Max)<br> Datepicker returned null,
                     day_start ", hDate($day_start), "( $day_start )
                     has been used</p>";
        }
    }

    // Sanity check because dates from the future are pointless (and untidy)
    // $latest_date holds the last (latest) entry in database.
    if ($pick_start > $latest_date) {
        if ($debug>='1') {
            echo "<p class=\"debug\"> pick_start ( $pick_start ) is > ".
                 "latest_date ( $latest_date ) </p>";
        }
        echo "<div class=\"alert\"><p>That is a future date; there are no ".
             "records!</p></div>";
        return;
    }

    // We have lift off! Start to actually work with the dates / times.

    $start_time = $pick_start;
    $day_end = $start_time + 86400;

    if ($shift_rain) {
        // Calculate start day for rain calcs - some recordings start at $start_label a.m.
        $today_nine = $start_time + $rainday_start; // $start_label am today
        $past_nine = $start_time - (86400 - $rainday_start); // $start_label am yesterday
        if ($debug>='1') {
            echo "Rain: Min Max today_nine = ". hDate($today_nine).
                 " ($today_nine) <br>";
            echo "Rain: Min Max past_nine = ". hDate($past_nine).
                 " ($past_nine) <br>";
        }
        // include up to the actual $start_label a.m. value
        if ($latest_date <= $today_nine) {
            //echo "} elseif ".hDate($latest_date)." ($latest_date < $today_nine) {". hDate($today_nine)."<br>";
            $rain_start = $past_nine;
            $rain_end = $past_nine + 86400;
            $t_span = "(since $start_label a.m.)";
            $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                       "Previous: Running Total&#10;starting ".
                       hDate($rain_start). "\">";
            // start from the $start_label a.m. value
        } elseif ($latest_date > $today_nine) {
            //echo "} elseif ".hDate($latest_date)." ($latest_date >= $today_nine) {". hDate($today_nine)."<br>";
            $rain_start =$today_nine;
            $rain_end = $today_nine + 86400;
            $t_span = "(since $start_label a.m.)";
            $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                        "Current: Running Total&#10;starting ".
                        hDate($rain_start). "\">";
        }
    } else {
        $rain_start = $start_time;
        $rain_end = $day_end;
        $t_span = "(since 12 a.m.)";
        $td_rain = "<td title=\"24 hour rain starting at&#10; midnight on: ".
                   hDate($rain_start). "\">";
    }
    if ($debug>='1') {
        echo "<p class=\"debug\">Min (Max) and Periodic Stats<br> start_time is now
             = pick_start ". hDate($pick_start). "( $pick_start ) and day_end is + 86400 "
             .hDate($day_end)." ( $day_end )</p>";
    }

    if ($php_dbase === 'mysql') {
        $stats_snap = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),min(outTemp),
            max(outTemp), min($app_temp), max($app_temp), min(dewPoint),
            max(dewPoint), min(outHumidity), max(outHumidity), avg(windSpeed),
            avg(windGust), avg(windDir), min(barometer), max(barometer) FROM archive
            WHERE dateTime >= $start_time and dateTime < $day_end;"
        );
        $rain_avg = $db->query(
            "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),
            sum(rain) FROM archive WHERE dateTime >= $rain_start
            and dateTime < $rain_end;"
        );

        $rain_row = $rain_avg->fetch_array();
        $stats_row = $stats_snap->fetch_array();
    } else {
        $stats_snap = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),min(outTemp),
            max(outTemp), min($app_temp), max($app_temp), min(dewPoint), max(dewPoint),
            min(outHumidity), max(outHumidity), avg(windSpeed), avg(windGust),
            avg(windDir), min(barometer), max(barometer) FROM archive WHERE dateTime
            >= $start_time and dateTime < $day_end;"
        );
        $rain_avg = $db->query(
            "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
           sum(rain) FROM archive WHERE dateTime >= $rain_start
           and dateTime < $rain_end;"
        );

        $rain_row = $rain_avg->fetchArray();
        $stats_row = $stats_snap->fetchArray();
    }
    // last date / data check. Do we have data for that date?
    $dbase_date = $stats_row[0];
    if ($dbase_date == "") {
        $read_able=date("d-M-Y H:i", $start_time);
        echo "<div class=\"alert\"><p>That date appears to be invalid, no data to
             display.<br> $start_time was given as the starting date ($read_able)
             </p></div></div></body></html>";
        return;
    }
    $this_day = date('d-M-Y', $pick_start);

    if ($debug >= 1) {
        echo "Min Max:<br> latest date = ", hDate($latest_date), " : day_start = ",
           hDate($day_start), "<br> start_time = ", hDate($start_time),
           " : day_end = ", hDate($day_end), "<br>rain_start = ",
           hDate($rain_start), " : rain_end = ", hDate($rain_end), "<br>";
    }

    // Finish Min (Max) grunt work
    //////////////////////////
    // Build Min (Max) Obs html
    // Create Min Max table headers - ugly layout but ok in html source
    echo "<!-- start snapshot summary observations -->
 <h2>Historical Observations for $this_day</h2>
 <h3><a title=\"Lowest and Highest observations for the nominated day.\">Min (Max) Stats</a></h3>
 <table class=\"wxdata\">
  <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh \n   $st_wind \n   $st_baro 
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    if ($debug >= 1) {
        $st_day = hDate($start_time);
        $st_day = "$st_day($start_time)";
        $en_day = hDate($day_end);
        $en_day = "$en_day($day_end)";
    } else {
        $st_day = "epochtimes available here if wxobs_debug = 1 in skin.conf";
        $en_day = "";
    }


    echo "<tr><td> $dbase_date<!-- $st_day : $en_day --></td>";
    $x = $tempConvert ($stats_row[1]);
    $minTemp = round($x, 1);
    $x = $tempConvert ($stats_row[2]);
    $maxTemp = round($x, 1);
    echo "<td>$minTemp ($maxTemp)</td>";
    $x = $tempConvert ($stats_row[3]);
    $minappTemp = round($x, 1);
    $x = $tempConvert ($stats_row[4]);
    $maxappTemp = round($x, 1);
    echo "<td>$minappTemp ($maxappTemp)</td>";
    $x = $tempConvert ($stats_row[5]);
    $mindew = round($x, 1);
    $x = $tempConvert ($stats_row[6]);
    $maxdew = round($x, 1);
    echo "<td>$mindew ($maxdew)</td>";
    $minhumid = round($stats_row[7], 0);
    $maxhumid = round($stats_row[8], 0);
    echo "<td>$minhumid ($maxhumid)</td>";
    ordinalCompass($stats_row[11], $debug);
    echo "<td> $compass</td>"; //wind direction ordinal
    list($x, $dec_plc) = $speedConvert ($stats_row[9], $wind_plc);
    $metWind = round($x, $dec_plc);
    echo "<td>$metWind</td>";
    list($x, $dec_plc) = $speedConvert ($stats_row[10], $wind_plc);
    $metGust = round($x, $dec_plc);
    echo "<td>$metGust</td>";
    $windDir = round($stats_row[11], 0);
    echo "<td>$windDir</td>"; //wind direction degrees
    list($x, $dec_plc) = $pressConvert ($stats_row[12], $press_plc);
    $minPress = round($x, $dec_plc);
    list($x, $dec_plc) = $pressConvert ($stats_row[13], $press_plc);
    $maxPress = round($x, $dec_plc);
    echo "<td>$minPress ($maxPress)</td>";
    list($x, $dec_plc) = $rainConvert ($rain_row[1], $rain_plc);
    $metRain = round($x, $dec_plc);
    echo "$td_rain $metRain</td></tr>";
    echo "</tbody></table>\n<!-- end snapshot summary observations -->\n";

    // Finish Min (Max) Stats
    //////////////////////////
    // Build Periodic daily stats html
    // Shows the (average) periodic values for a given (arch_interval) snapshot,
    // averages and rainfall total for that period.
    // Create Min Max table headers - ugly layout but ok in html source
    echo "<!-- start snapshot observations -->
 <h3><a title=\"Time stamped observations throughout the nominated day.\">Periodic Stats $iflag</a></h3>
 <table class=\"wxdata\">
 <thead>
  <tr>
   $st_date \n   $st_temp
   <th class=\"wxheadr\" rowspan=\"2\">$appLabel<br>$app_label</th>
   $st_dew \n   $st_rh
   $delta_label
   $st_wind \n   $st_baro
   $st_rain $tflag Rain<br>$t_span<br>$unit.label.rain</a></th>
  </tr>
  <tr>
   $st_comp \n   $st_spd \n   $st_gust \n   $st_degr
  </tr>
 </thead><tbody>";

    // extract data for each ext_interval - reverse order (latest at top)
    $c_ycle=(86400/$ext_interval);
    // this may appear to include an extra interval - midnight to midnight.
    // but we're counting panels, not fence posts. ie: It's correct.
    for ($t=$c_ycle; $t>=0; $t--) {
        $start_time = $pick_start + ($t*'$ext_interval');
        $end_interval = $start_time + '$arch_interval';

        if ($shift_rain) {
            // Calculate start day for rain calcs
            // Some  records start at $start_label a.m.
            $today_nine = $pick_start + $rainday_start; // $start_label am today
            $past_nine = $pick_start - (86400 - $rainday_start); // $start_label am yesterday
            // include up to the actual $start_label a.m. value
            if ($start_time <= $today_nine) {
                 $rain_start = $past_nine;
                 $rain_end = $end_interval;//$past_nine + 86400;
                 $td_rain = "<td style=\"background-color:#aeaeae\" title=\"".
                            "Previous: Running Total&#10;starting ".
                            hDate($rain_start)."\">";
                // start from the $start_label a.m. value
            } elseif ($start_time > $today_nine) {
                 $rain_start =$today_nine;
                 $rain_end = $end_interval;//$today_nine + 86400;
                 $td_rain = "<td style=\"background-color:#d6d6d6\" title=\"".
                            "Current: Running Total&#10;starting ".
                            hDate($rain_start). "\">";
            }
        } else {
            $rain_start = $pick_start;
            $rain_end = $end_interval;
            $t_span = "(since 12 a.m.)";
            $td_rain = "<td title=\"Rain starting at&#10;midnight on: ".
                       sDate($rain_start). "\">";
        }

        if ($debug >= 6) {
            echo "Periodic Stats :<br> latest date = ", hDate($latest_date),
                 " : day_start = ", hDate($day_start), "<br> start_time = ",
                 hDate($start_time), " : end_interval = ", hDate($end_interval),
                 "<br>rain_start = ", hDate($rain_start), " : rain_end = ",
                 hDate($rain_end), "<br>";
        }

        // mysql will return the date as dd-mm-yyyy (as per the db->query statements
        // following) adjust if desired.
        // sqlite will return the date as yyyy-mm-dd (default and appears to be locked?)
        if ($php_dbase === 'mysql') {
               $results = $db->query(
                   "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'), avg(outTemp),
                   avg($app_temp), avg(dewPoint), avg(outHumidity), avg(windSpeed),
                   avg(windGust), avg(windDir), avg(barometer), sum(rain) FROM
                   archive WHERE dateTime >= $start_time and dateTime < $end_interval;"
               );
                $rain_avg = $db->query(
                    "SELECT FROM_UNIXTIME(datetime, '%d-%m-%Y %H:%i'),
                    sum(rain) FROM archive WHERE dateTime >= $rain_start
                    and dateTime < $rain_end;"
                );

               $row = $results->fetch_array();
               $rain_row = $rain_avg->fetch_array();
        } else {
            $results = $db->query(
                "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
                avg(outTemp), avg($app_temp), avg(dewPoint), avg(outHumidity),
                avg(windSpeed), avg(windGust), avg(windDir), avg(barometer),
                total(rain) FROM archive WHERE dateTime>= $start_time and
                dateTime< $end_interval;"
            );
            $rain_avg = $db->query(
                "SELECT datetime(min(dateTime), 'unixepoch', 'localtime'),
               sum(rain) FROM archive WHERE dateTime >= $rain_start
               and dateTime < $rain_end;"
            );

            $row = $results->fetchArray();
            $rain_row = $rain_avg->fetchArray();
        }
        // temporary debug for wind / CALM substitution
        // echo "$row[0], temp = $row[1], speed = $row[5],  direction = $row[7]<br>";
        if ($tr_bgrnd != "wxhint") {
            $tr_bgrnd = "wxhint";
        } else {
            $tr_bgrnd = "wxwht";
        }

        $dbase_date = $row[0];

        // Values enclosed in comments within generated html tables
        if ($debug >= 1) {
            $st_day = hDate($start_time);
            $st_day = "$st_day($start_time)";
            $en_day = hDate($end_interval);
            $en_day = "$en_day($end_interval)";
        } else {
            $st_day = "";
            $en_day = "";
        }

        if ($dbase_date != "") {
            echo "<tr class=\"$tr_bgrnd\"><td> $dbase_date <!-- $st_day : $en_day -->";
            $x = $tempConvert ($row[1]);
            $metTemp = round($x, 1);
            echo "</td><td>$metTemp</td>";
            $x = $tempConvert ($row[2]);
            $metappTemp = round($x, 1);
            echo "<td>$metappTemp</td>";
            $x = $tempConvert ($row[3]);
            $metdew = round($x, 1);
            echo "<td>$metdew</td>";
            $humid = round($row[4], 0);
            echo "<td>$humid</td>";
            if ($want_delta) {
                deltaT($row[1], $row[4], $row[8], $debug);
                echo "<td>$dT</td>";
                if ($debug=='3') {
                    echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
                    echo "delta_data is '$dT'</p>";
                }
            } else {
                if ($debug=='3') {
                    echo "<p class=\"debug\">want_delta is $want_delta so ...<br>";
                    echo "delta_data is 'null'</p>";
                }
            }
            ordinalCompass($row[7], $debug);
            echo "<td>$compass</td>"; //wind direction ORDINAL
            list($x, $dec_plc) = $speedConvert ($row[5], $wind_plc);
            $metWind = round($x, $dec_plc);
            echo "<td>$metWind</td>";
            list($x, $dec_plc) = $speedConvert ($row[6], $wind_plc);
            $metGust = round($x, $dec_plc);
            echo "<td>$metGust</td>";
            $windDir = round($row[7], 0);
            echo "<td>$windDir</td>"; //wind direction degrees
            list($x, $dec_plc) = $pressConvert ($row[8], $press_plc);
            $metPress = round($x, $dec_plc);
            echo "<td>$metPress</td>";
            list($x, $dec_plc) = $rainConvert ($rain_row[1], $rain_plc);
            $metRain = round($x, $dec_plc);
            echo "$td_rain $metRain</td></tr>";
        }
    }
    echo "</tbody></table></div><!-- end snapshot observations -->";
    $db->close();
    unset($db);

    $end = microtime(true);
    $creationtime = round(($end - $start), 2);
    echo "<p class=\"footnote\">Time taken to generate page was $creationtime seconds.</p>";
    ?>
