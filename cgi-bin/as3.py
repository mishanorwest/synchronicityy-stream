#!/usr/bin/python3
from atlas import LookupCity, TimeChangeLookup, DupCity, CountryAtIndex, CityAtIndex
from sys import argv, exit
from time import strftime

def Lookup(date, time, place):
	i = place.find(',')
	if i == -1:
		print("No state/country")
		return 
	st = place[i+1:]
	x  = LookupCity(place)
	if x[1] == 'Error':
		print(x[0])
		return
	elif x[1] == 'Duplicate':
		n = int(x[0][:2])  # number of  duplicates
		List = []
		for i in range(n):
			y = DupCity(i+1, date, time)
			List.append(y)
		print("Duplicate Cities:")
		n = 1
		for l in List:
			if l[5][0] == '*':
				print("%d) %s in county %s (Most likely)" % (n, l[5][1:], l[4]))
			else:
				print("%d) %s in county %s" % (n, l[5], l[4]))
			n += 1
		s = raw_input("Choose by number which duplicate city to use: ")
		n =  int(s) - 1 
		if n >= 0 and n < len(List):
			print("For %s on %s %s:\nLocation %s %s %s (%s)" % (place, date, time, List[n][0], List[n][1], List[n][3], List[n][2]))
		else:
			print("Error")
	else:
		lat = x[0]
		longg = x[1]
		dist = x[2]
		citynum = x[4]
		city,index = CityAtIndex(citynum)
		st,abbrev = CountryAtIndex(x[3])
		a = TimeChangeLookup(date, time) # change date string
		if a[1] == 'Error':
			print(a[0])
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
			print("For %s, %s on %s %s:\nLocation %s %s %s (%s)\nDistrict: %s" % (s, st, date, time, lat, longg, zname, tz, t))

while 1:

	date = input('Date: ')
	if date == '':
		date = strftime("%m/%d/%Y") #  use today	
	elif date[0] == 'q' or date[0] == 'Q':
		exit(0)
	time = input('Time: ')
	if time == '':
		time = strftime('%H:%M')   # use now
	place = input('Place: ')
	Lookup(date, time, place)

