import random 
import sys


max_campaigns = 100

geo = []
file = open("../data/world_cities.csv", "r")
idx = 1
for line in file:
    row = line.split(',')
    # For now
    if row[5] != "Russia":
        continue
    geo.append([idx, row[0], row[5]])
    idx+=1
file.close()

file = open("../data/geo", "w")

for g in geo:
    file.write("%d\t%s\t%s\n" % (g[0], g[1], g[2]))
    
file.close()
size = [
    [88,31],[120,60],[120,240],[120,600],[125,125],[160,600],[180,150],
    [200,200],[200,446],[220,90],[234,60],[240,133],[240,400],[250,250],
    [250,360],[292,30],[300,31],[300,50],[300,100],[300,250],[300,600],
    [300,1050],[320,50],[320,100],[336,280],[468,60],[580,400],[728,90],
    [750,100],[750,200],[750,300],[930,180],[950,90],[960,90],[970,66],
    [970,90],[970,250],[980,90],[980,120]
]

position = [0, 1, 2]
max_bid = [1, 1000]
codes = ["""<script>alert("code1!");</script>""", """<script>alert("code2!");</script>"""]

file = open("../data/geo_campaign", "w")
for geo_id, city, country in geo:
    max_targetings = random.randint(1, max_campaigns/10)
    start_pos = random.randint(1, max_campaigns-max_targetings)
    for i in range(start_pos, start_pos+max_targetings+1):
        file.write("%d\t%d\n" % (geo_id, i))
file.close()


max_ad = 1
max_ads_in_campaign = 30

file = open("../data/ads", "w")
for campaign_id in range(1, max_campaigns+1):    
    ads_in_campaign = random.randint(1, max_ads_in_campaign)
    
    for ad_id in range(max_ad, max_ad+ads_in_campaign+1):
        rand_size = random.choice(size),
        file.write("%u\t%u\t%u\t%u\t%u\t%u\t%s\n" % (
            ad_id, 
            campaign_id,
            rand_size[0][0],
            rand_size[0][1],
            random.choice(position),
            random.randint(max_bid[0], max_bid[1]),
            random.choice(codes)
        ))
    max_ad+=ads_in_campaign+1
file.close()

file = open("../data/ad_geo", "w")
for ad_id in range(1, max_ad+1):
    max_targetings = random.randint(1, len(geo)/3)
    start_pos = random.randint(1, len(geo))
    for i in range(start_pos, start_pos+max_targetings+1):
       file.write("%d\t%d\n" % (ad_id, geo[i%len(geo)][0]))
file.close()