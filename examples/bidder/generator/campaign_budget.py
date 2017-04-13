import random 
import sys
import argparse

#format 
# id\tday_budget\day budget spentday_show_limit\tday_click_limit
campaigns = 1000
budget = [0, 10000000000]
show_limit = [0, 10000000]
click_limit = [0, 10000]

for campaign_id in range(1, campaigns+1):
    campaign_budget = random.randint(budget[0], budget[1])
    campaign_show_limit = 0
    if random.choice([True, False]):
        campaign_show_limit = random.randint(show_limit[0], show_limit[1])
    campaign_click_limit = 0
    if random.choice([True, False]):
        campaign_click_limit = random.randint(click_limit[0], click_limit[1])
    print """%(id)s\t%(budget)s\t%(spent)s\t%(show_limit)s\t%(click_limit)s""" % {
       'id' : campaign_id,
       'budget' : campaign_budget,
       'spent': random.randint(0, campaign_budget),
       'show_limit' : campaign_show_limit,
       'click_limit' : campaign_click_limit
    }
