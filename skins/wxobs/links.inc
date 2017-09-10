## forecast module for standard skin
## Copyright Tom Keffer, Matthew Wall
## See LICENSE.txt for your rights
##
## Australian Index Forecast panel
## Included in index.html.tmpl
##
## Depending of your location and XML file in skin.conf
## you will need to adjust this file in a few places
## From Responsive skin with all credits to Darryn Cape


#import dateutil.parser

#errorCatcher Echo

<div id='extlink_widget' class="widget">
  <div class="widget_title">
   <a class="widget_control_left"
        onclick="toggle_widget('extlink')">External Links</a>
        &nbsp;
   <a class="widget_control"
     onclick="toggle_widget('extlink')">&diams;</a>
  </div>
  <div class="widget_contents">
   <div id="extlink_details">
   <table>
   <tr>
   <td>
   &nbsp;<img src="favicon.ico" alt="">
   </td>
    <td>
    wxobs - <a href="index.php?datepicker=$day.dateTime.format('%d-%m-%Y')&submitDate=Show+records" target="_blank">Daily summaries</a>
    </td>
   </tr>
   </table>
   </div>
  </div>
</div>

