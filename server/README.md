# timezoned - The Timezone Daemon

>If you do not plan to run your own timezone information server, you do not need anything in this directory...

This is a brutally ugly hack that serves timezone information via UDP port 2342. To use it, try the following on a unix machine with PHP installed:

* Create a user called 'timezoned'

* Copy the 'update' and 'server' scripts to this user's homedir, and change the #! line at the beginning of the server script to point to the PHP binary on the system 

* Log in or su to the timezoned user, make the 'update' script executable and run it.

* Make the 'server' script executable and run it. (Make sure the server accepts packets on 2342 UDP.)

* Test by running `nc -u <ip or domain name> 2342` on some other system and then typing a zone (like "Europe/London") followed by Ctrl-D. You should get the POSIX information for that zone.

* If that works, you may (on a FreeBSD machine) use the 'timezoned' script by placing it in /usr/local/etc/rc.d to start the server automatically. On other systems, you'll have to figure out how to start it automatically when the server reboots.

* Run update script and then restart the server periodically to stay up on timezone updates.