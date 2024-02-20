#!/usr/bin/env python3

import random 
import sys
#import argparse

#format 
# id\tday_budget\day budget spentday_show_limit\tday_click_limit
campaigns = 1000
budget = [100, 10000000000]
metrics = {
            'CPM': {'id': 1, 'limit': [0, 1000000]}, 
            'CPC': {'id': 2, 'limit': [0, 1000]}
          }

for campaign_id in range(1, campaigns+1):
    campaign_budget = random.randint(budget[0], budget[1])
    metric = random.choice(list(metrics)) #make it run on both python 2.x and 3.x
    print("%(id)s\t%(budget)s\t%(spent)s\t%(metric)s\t%(limit)s" % {
       'id' : campaign_id,
       'budget' : campaign_budget,
       'spent': random.randint(0, campaign_budget),
       'metric' : metrics[metric]['id'],
       'limit' : random.randint(metrics[metric]['limit'][0], metrics[metric]['limit'][1])
    })

