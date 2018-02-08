import random

max_campaigns = 100

required = {'ref': ['www.coinbase.com'], 'size': [300, 100], 'campaigns': []}

referer_list = ['www.coinbase.com', 'www.coindesk.com' , 'www.cointelegraph.com', 'www.coinmarketcap.com']

ico_referers = []

for n, ref in enumerate(referer_list):
    ico_referers.append([n, ref])

file = open("../data/ico_referers", "w")

for n, ref in ico_referers:
    file.write("%d\t%s\n" % (n, ref))

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
code = """<script>alert(" ad %d!");</script>"""

file = open("../data/ico_campaign", "w")
for ref_id, domain in ico_referers:
    max_targetings = random.randint(1, max_campaigns/10)
    start_pos = random.randint(1, max_campaigns-max_targetings)
    for i in range(start_pos, start_pos+max_targetings+1):
        file.write("%d\t%d\n" % (ref_id, i))
    if domain == required['ref'][0] :
        for i in range(start_pos, start_pos+max_targetings+1):
            required['campaigns'].append(i)
file.close()


max_ad = 1
max_ads_in_campaign = 30
file = open("../data/ico_ads", "w")
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
            code % (ad_id)
        ))
    if campaign_id in required['campaigns']:
        ads_in_campaign += 1
        file.write("%u\t%u\t%u\t%u\t%u\t%u\t%s\n" % (
            max_ad+ads_in_campaign,
            campaign_id,
            required['size'][0],
            required['size'][1],
            random.choice(position),
            random.randint(max_bid[0], max_bid[1]),
            code % (max_ad+ads_in_campaign)
        ))
    max_ad+=ads_in_campaign+1
file.close()
