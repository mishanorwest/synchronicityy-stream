#!/usr/bin/python3
# 9-2-3 lookup location for dtc
from atlas import LookupCity, TimeChangeLookup, DupCity, CountryAtIndex, CityAtIndex
import cgi, cgitb
from sys  import stderr

lat =  ''
longg = ''
tz = ''
zname = ''
dups = []

def Lookup(date, time, place):
	global lat, longg, tz, zname

	i = place.find(',')
	if i == -1:
		#print("No state/country")
		lat = 'Error'
		longg = "No state/country"
		return 
	st = place[i+1:]
	x  = LookupCity(place)
	if x[1] == 'Error':
		#print(x[0])
		lat = x[1]
		longg = x[0]
		return
	elif x[1] == 'Duplicate':
		#lat = 'Error'
		#longg = 'DupCities'
		#return
		n = int(x[0][:2])  # number of  duplicates
		List = []
		for i in range(n):
			y = DupCity(i+1, date, time)
			List.append(y)
		#print("Duplicate Cities:")
		n = 1
		lat = 'Dups'
		for l in List:
			if l[5][0] == '*':
				#print(("%d) %s in county %s (Most likely)" % (n, l[5][1:], l[4])))
				s = ("%d) %s in county %s (Most likely)" % (n, l[5][1:], l[4]))
				dup.append(s)
			#else:
				#print(("%d) %s in county %s" % (n, l[5], l[4])))
				s = ("%d) %s in county %s" % (n, l[5], l[4]))
				dup.append(s)
			n += 1
		#s = input("Choose by number which duplicate city to use: ")
		n =  int(s) - 1 
		#if n >= 0 and n < len(List):
			#print(("For %s on %s %s:\nLocation %s %s %s (%s)" % (place, date, time, List[n][0], List[n][1], List[n][3], List[n][2])))
		#else:
			#print("Error")
	else:
		lat = x[0]
		longg = x[1]
		dist = x[2]
		citynum = x[4]
		city,index = CityAtIndex(citynum)
		st,abbrev = CountryAtIndex(x[3])
		a = TimeChangeLookup(date, time) # change date string
		if a[1] == 'Error':
			#print(a[0])
			lat = a[1]
		else:
			tz = a[1]
			zname = a[0]
			if abbrev == 'AFG' or abbrev == 'ARMENI' or abbrev == 'AZERBA' or abbrev == 'BAHR' or abbrev == 'BELARU' or abbrev == 'BOSINA' or abbrev == 'BULG' or abbrev ==  'CAMB' or abbrev ==  'CROAT' or abbrev ==  'CZEC' or abbrev ==  'EG' or abbrev ==  'GEORGI' or abbrev ==  'IR' or abbrev ==  'IQ' or abbrev ==  'ISRL' or abbrev ==  'JOR' or abbrev ==  'KAZAKH' or abbrev ==  'KYRGYZ' or abbrev ==  'LEB' or abbrev ==  'LY' or abbrev ==  'LITH' or abbrev ==  'MACED' or abbrev ==  'MOLDOV' or abbrev ==  'MONG' or abbrev ==  'OM' or abbrev ==  'QA' or abbrev ==  'RU' or abbrev ==  'SAUDI' or abbrev ==  'SLOVAK' or abbrev ==  'SLOVEN' or abbrev ==  'SUDAN' or abbrev ==  'SY' or abbrev ==  'TAJIKI' or abbrev ==  'TURKME' or abbrev ==  'UKRAIN' or abbrev ==  'UAE' or abbrev ==  'UZBEK' or abbrev ==  'YE' or abbrev ==  'YUG':
				cp1250 = 1
			else:
				cp1250 = 0
			#if cp1250 == 1:
				#s = str(city, 'cp1250')
				#t = str(dist, 'cp1250')
				#s = city.decode('cp1250')
				#t = dist.decode('cp1250')
			#else:
				#s = str(city, 'latin1')
				#t = str(dist, 'latin1')
				#s = city.decode('latin1')
				#t = dist.decode('latin1')
			s = city
			t = dist
			#print("For %s, %s on %s %s:\nLocation %s %s %s (%s)\nDistrict: %s" % (s.encode('utf-8'), st, date, time, lat, longg, zname, tz, t.encode('utf-8')))
			#print(("For %s, %s on %s %s:\nLocation %s %s %s (%s)\nDistrict: %s" % (s, st, date, time, lat, longg, zname, tz, t)))

forms = cgi.FieldStorage()
place = ''
if 'Place' in forms:
	#place = forms['Place'].value
	place = forms.getvalue('Place')
	#date = forms['Date'].value
	date = forms.getvalue('Date')
	#time = forms['Time'].value
	time = forms.getvalue('Time')
if 'email' in forms:
	email = forms.getvalue('email')
if 'tid' in forms:
	tid = forms.getvalue('tid')
#place='Seattle,WA'
#date='10/24/1946'
#time='2:07AM'
#(lat, long, zone, tz) = atlas(place, date, time)
if place != '':
	Lookup(date, time, place)
else:
	lat = 'Error'
	longg = '2'
#i = tz.find(' ')
#if i != -1:
	#tz = tz[:i]
#print('One', file=stderr)
print("Content-Type: text/html")
print() 
if lat == 'Error':
	n = int(longg)
	if n == 1:
		print(("Error=Location %s not found" % place))
	elif n == 2:
		print(("Error=Missing City"))
	elif n == 3:
		print(("Error=No state/country given for %s" % place))
	elif n == 4:
		print(("Error=State/country %s not found" % place))
elif lat == 'Dups':
	print('Dulicate Cities:')
	for i in dups:
		#(lat, longg, zone, tz, county, place) = atlas_dups(-1)
		#print(("Place%d=%s,%s,%s,%s,%s" % (i+1, lat, longg, zone, county, place)))
		print(i)
else:
	print(("Latitude=%s" % lat))
	print(("Longitude=%s" % longg))
	#print(("TZ=%s" % zone))
	print(("TZ=%s" % zname))
	print(("Zone=%s" % tz))
