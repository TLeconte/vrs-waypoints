# VRS Waypoints
Support for RNAV waypoints in Virtual Radar Server.

![Example](example.png)

## Setup
  * Create wptgen programm : in src directory execute make command
  * Create waypoints.html file with: ./src/wptgen -f data/fix.dat -n data/nav.dat 48.5 1.5 > waypoints.html
  * Install the [Custom Content Plugin](http://www.virtualradarserver.co.uk/Documentation/CustomContent/Default.aspx)
  * Set "Site root folder" to "Images" folder
  * Inject "waypoints.html" and "vrs-waypoints.html" to all addresses at End of HEAD

  ![Setup](setup.png)
