####################################################################
from flask import Flask
from flask import render_template
from flask import request
import json


app = Flask(__name__)



@app.route("/")
def line_chart():

    with open("data.json") as json_file:
        data = json.load(json_file)


    timestamp_list_1 = []
    moist_list_1 = []
    temp_list_1 = []

    for item in data['basil']:
        timestamp_list_1.append(item['timestamp'])
        moist_list_1.append(item['moist'])
        temp_list_1.append(item['temp'])
    print(moist_list_1)
    print(timestamp_list_1)


    timestamp_list_2 = []
    moist_list_2 = []
    temp_list_2 = []

    for item in data['peppermint']:
        timestamp_list_2.append(item['timestamp'])
        moist_list_2.append(item['moist'])
        temp_list_2.append(item['temp'])
    print(moist_list_2)
    print(timestamp_list_2)



    t_legend_plant_1 = 'Temperatures Basil'
    m_legend_plant_1 = 'Moisture Basil'
    temperatures_plant_1 = temp_list_1[-24:]
    moistures_plant_1 = moist_list_1[-24:]
    times_plant_1 = timestamp_list_1[-24:]

    t_legend_plant_2 = 'Temperatures Peppermint'
    m_legend_plant_2 = 'Moisture Peppermint'
    temperatures_plant_2 = temp_list_2[-24:]
    moistures_plant_2 = moist_list_2[-24:]
    times_plant_2 = timestamp_list_2[-24:]


    return render_template('line_chart.html',
    t_values_plant_1=temperatures_plant_1,
    m_values_plant_1=moistures_plant_1,
    labels_plant_1=times_plant_1,
    t_legend_plant_1=t_legend_plant_1,
    m_legend_plant_1=m_legend_plant_1,
    t_values_plant_2=temperatures_plant_2,
    m_values_plant_2=moistures_plant_2,
    labels_plant_2=times_plant_2,
    t_legend_plant_2=t_legend_plant_2,
    m_legend_plant_2=m_legend_plant_2,
    )



@app.route('/postjson', methods=['GET', 'POST', 'DELETE', 'PUT'])
def postJsonHandler():
    content = request.get_json()
    print (content)

    #f = open("data.json", "x")

    with open('data.json') as json_file:
        data = json.load(json_file)
        sensorX = content

        temp = data['arduino_sensors']

        # appending data to emp_details
        temp.append(sensorX)


    with open('data.json','w') as f:
        json.dump(data, f, indent=4)

    return 'JSON posted'



if __name__ == "__main__":
    app.run(debug=True)




