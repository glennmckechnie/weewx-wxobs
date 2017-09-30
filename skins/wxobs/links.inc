## forecast module for standard skin
## Copyright Tom Keffer, Matthew Wall
## See LICENSE.txt for your rights
##
## Daily summary panel
##
## Links to a report page that can generates daily summaries
## for the current day, or from a selected date


#import dateutil.parser

#errorCatcher Echo

<div id='wxobs_widget' class="widget">
  <div class="widget_title">
   <a class="widget_control_left"
        onclick="toggle_widget('wxobs')">&diams;</a>
        <a href="wxobs/index.php?datepicker=$day.dateTime.format('%d-%b-%Y')&submitDate=Show+records" target="_blank">Daily Summary</a> (wxobs)
        &nbsp;
   <a class="widget_control"
     onclick="toggle_widget('wxobs')">&diams;</a>
  </div>
  <div class="widget_contents">
  <center>
  Shows <a href="wxobs/index.php?datepicker=$day.dateTime.format('%d-%b-%Y')&submitDate=Show+records" target="_blank">todays data.</a><br>
  Or select a date to<br>
  extract past data directly<br>
  from the weewx database.
  </center>
  </div>
</div>

