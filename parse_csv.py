import plotly.offline as py
import plotly.graph_objs as go
import plotly.io as pio
import csv
import math
import os

dataSingleRead = []
data = []

mapTypes = ['Sequential', 'Concurrent Bucket Locks',
            'Hardware Lock Elision', 'Folly Concurrent', 'Folly Atomic']

with open('run.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='"')

    for row in reader:
        test_name = row[0]
        time_in_ns = float(row[2])

        test_name_split = test_name.split('/')

        test_type = test_name_split[1]
        numBuckets = int(test_name_split[2])
        numKeys = int(test_name_split[3])
        mapTypeId = int(test_name_split[4])
        percentReads = int(test_name_split[5])
        loadFactor = float(test_name_split[6])/100
        numThreads = int(test_name_split[8].split(':')[1])
        mapType = mapTypes[int(math.log(mapTypeId, 2))]

        entry = {'testType': test_type, 'mapType': mapType, 'numBuckets': numBuckets, 'numKeys': numKeys,
                 'percentReads': percentReads, 'numThreads': numThreads, 'loadFactor':loadFactor, 'time_in_ns': time_in_ns}
        data.append(entry)

 
def plot(primary, others):
    unique_primaries = sorted({entry[primary] for entry in data})
    # print(unique_primaries)

    unique_others = sorted(
        {(entry[others[0]], entry[others[1]], entry[others[2]]) for entry in data})
    # print(unique_others)

    for others_val in unique_others:
        graph_data = []
        for primary_val in unique_primaries:
            vals = []
            for mapType in mapTypes:
                matching_entries = list(filter(lambda entry: entry['mapType'] == mapType and entry[primary] == primary_val and
                                               entry[others[0]] == others_val[0] and entry[others[1]] == others_val[1] and entry[others[2]] == others_val[2], data))
                if len(matching_entries) != 1:
                    # print(primary, others, primary_val, others_val, matching_entries)
                    break
                assert(len(matching_entries) == 1)

                matching_entry = matching_entries[0]
                vals.append(matching_entry['time_in_ns'])

            graph_data.append(go.Bar(x=mapTypes, y=vals, name=primary_val))

        title = '{} with {}={}, {}={}, and {}={}'.format(primary, others[0], others_val[0], others[1], others_val[1], others[2], others_val[2])

        if len(graph_data) > 0:
            layout = go.Layout(barmode='group', title=title, yaxis=dict(title='time in ns'))
            fig = go.Figure(data=graph_data, layout=layout)
            # py.plot(fig, filename='grouped-bar')
            pio.write_image(fig, 'plots/{}.png'.format(title))


plot('percentReads', ('numKeys', 'loadFactor', 'numThreads'))
plot('numKeys', ('percentReads', 'loadFactor', 'numThreads'))
plot('loadFactor', ('numKeys', 'percentReads', 'numThreads'))
plot('numThreads', ('numKeys', 'loadFactor', 'percentReads'))
