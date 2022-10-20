#!/usr/bin/python3
# 9-2-3 lookup location for dtc
from atlas import atlas, atlas_dups
import cgi

forms = cgi.FieldStorage()
place = forms['Place'].value
date = forms['Date'].value
time = forms['Time'].value
if forms.has_key('email'):
	email = forms['email'].value
if forms.has_key('tid'):
	tid = forms['tid'].value
#place='Seattle,WA'
#date='10/24/1946'
#time='2:07AM'
(lat, long, zone, tz) = atlas(place, date, time)
#i = tz.find(' ')
#if i != -1:
    #tz = tz[:i]
print "Content-Type: text/html"
print 
if lat == 'Error':
    n = int(long)
    if n == 1:
        print "Error=Location %s not found" % place
    elif n == 2:
        print "Error=Missing City"
    elif n == 3:
        print "Error=No state/country given for %s" % place
    elif n == 4:
        print "Error=State/country %s not found" % place
elif lat == 'Duplicate Cities':
    for i in range(-int(long)):
        (lat, long, zone, tz, county, place) = atlas_dups(-1)
	print "Place%d=%s,%s,%s,%s,%s" % (i+1, lat, long, zone, county, place)
else:
    print "Latitude=%s" % lat
    print "Longitude=%s" % long
    print "TZ=%s" % zone
    print "Zone=%s" % tz
