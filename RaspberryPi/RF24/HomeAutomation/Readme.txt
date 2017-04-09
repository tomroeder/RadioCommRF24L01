homesensord - Home sensor daemon
--------------------------------


Listens to RF24 radio signal from arduino radio sensor modules
Writes data into sqlite database
Start as root
Find log messages of homesensord in /var/log/messages 
For autostart insert "/mydirectory/homesensord&" in /etc/rc.local (starts as root)
For autostart of Rails in production mode insert "/path_to_rails_app/bin/rails server -e production &"  in /etc/rc.local
