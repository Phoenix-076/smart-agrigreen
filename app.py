from flask import Flask, render_template, request, redirect, url_for, session,jsonify
import firebase_admin
from firebase_admin import credentials, auth, db
from datetime import datetime

app = Flask(__name__)
app.secret_key = 'thisismyfourthyearcapstoneproject'

# Initialize Firebase Admin once
cred = credentials.Certificate('firebasekey.json')
default_app = firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://smart-agrigreen-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

@app.route('/')
def index():
    return render_template('login.html')

@app.route('/signup')
def signup():
    return render_template('signup.html')

@app.route('/login', methods=['POST'])
def login():
    email = request.form['email']
    password = request.form['password']
    try:
        # Client-side handles actual authentication; here you manage the session
        user = auth.get_user_by_email(email)
        session['user_id'] = user.uid
        return redirect(url_for('dashboard'))
    except firebase_admin.auth.AuthError as e:
        return "Login failed! Please check your credentials."

@app.route('/set_session', methods=['POST'])
def set_session():
    data = request.get_json()
    print("data: ",data)
    session['user_id'] = data['user_id']
    session['user_name'] = data['user_name']
    return jsonify({"status": "success", "message": "Session set successfully."})

@app.route('/forgot_password')
def forgot_password():
    return render_template('forgot_password.html')
@app.route('/dashboard')
def dashboard():
    print("Session Data:", session)  # Check what's in the session
    if 'user_id' not in session:
        print("Redirecting because user_id not found in session")
        return redirect(url_for('index'))
    user_name = session.get('user_name')
    return render_template('dashboard.html',user_name=user_name)

@app.route('/settings')
def settings():
    if 'user_id' not in session:
            print("Redirecting because user_id not found in session")
            return redirect(url_for('index'))
    user_name = session.get('user_name')
    return render_template('settings.html',user_name=user_name)

def filter_history_by_date(sensor_data, actuator_data, date_str):
    filtered_history = []

    for timestamp, sensor_entry in sensor_data.items():
        try:
            # Parse the custom timestamp format
            timestamp_datetime = datetime.strptime(timestamp, "%Y%m%d_%H%M%S")
            entry_date = timestamp_datetime.strftime("%Y-%m-%d")  # Convert to desired date format
            entry_time = timestamp_datetime.strftime("%I:%M %p")  # Convert to 12-hour format

            if entry_date == date_str:
                # Extract sensor data
                soil_moisture1 = sensor_entry.get('soil_moisture_value1', 'N/A')
                soil_moisture2 = sensor_entry.get('soil_moisture_value2', 'N/A')
                temperature = sensor_entry.get('temperature', 'N/A')

                # Extract actuator data
                actuator_entry = actuator_data.get(timestamp, {})
                pump_status = actuator_entry.get('pump_status', 'N/A')
                pump_status = "ON" if pump_status == 1 else "OFF"
                fan_status = actuator_entry.get('fan_status', 'N/A')
                fan_status = "ON" if fan_status == 1 else "OFF"

                filtered_history.append({
                    'time': entry_time,
                    'soil_moisture1': soil_moisture1,
                    'soil_moisture2': soil_moisture2,
                    'temperature': temperature,
                    'pump_status': pump_status,
                    'fan_status': fan_status
                })
        except ValueError as e:
            print(f"Error parsing timestamp: {timestamp}. Error: {e}")
            continue

    return filtered_history



@app.route('/history', methods=['GET', 'POST'])
def history():
    if 'user_id' not in session:
        return redirect(url_for('index'))

    user_name = session.get('user_name')

    # Fetch data from Firebase Realtime Database
    try:
        sensor_ref = db.reference('/history/sensor/plot1')
        actuator_ref = db.reference('/history/actuator/plot1')
        sensor_data = sensor_ref.get()  # Fetch sensor history
        actuator_data = actuator_ref.get()  # Fetch actuator history
    except Exception as e:
        print("Error fetching data from Firebase:", e)
        sensor_data = None
        actuator_data = None

    # Get the current date or the user-selected date
    if request.method == 'POST':
        selected_date = request.form.get('date')
    else:
        selected_date = datetime.now().strftime('%Y-%m-%d')  # Current date

    # Filter history data by the selected date
    history_list = filter_history_by_date(sensor_data, actuator_data, selected_date) if sensor_data and actuator_data else None

    return render_template('history.html', user_name=user_name, history_list=history_list, selected_date=selected_date)

@app.route('/get_soil_moisture')
def get_soil_moisture():
    # Fetch the current soil moisture data from Firebase
    current_ref = db.reference('current/sensor/plot1/')
    current_data = current_ref.get()
    soil_moisture_value1 = current_data['soil_moisture_value1']
    soil_moisture_value2 = current_data['soil_moisture_value2']
    temperature = current_data['temperature']
    # Return the soil moisture value as a JSON response
    return jsonify({'moisture1': soil_moisture_value1,'moisture2': soil_moisture_value2, 'temperature': temperature})

@app.route('/fetch_pump_frequency', methods=['GET'])
def fetch_pump_frequency():
    try:
        # Fetch pump frequency data from Firebase
        pump_ref = db.reference('usage_statistics/daily_pump_frequency')
        pump_data = pump_ref.get()  # Fetch the data

        # Debugging log to check the fetched data
        print(f"Fetched pump data: {pump_data}")

        if not pump_data:
            return jsonify({"error": "No data found"}), 404  # Return a 404 if no data is found

        return jsonify(pump_data)  # Return the data in JSON format
    except Exception as e:
        print(f"Error fetching pump frequency data: {e}")
        return jsonify({"error": str(e)}), 500  # Return the error in the response



@app.route('/logout')
def logout():
    session.pop('user_id', None)
    return redirect(url_for('index'))

@app.route('/plant_yield')
def plant_yield():
    if 'user_id' not in session:
        print("Redirecting because user_id not found in session")
        return redirect(url_for('index'))
    user_name = session.get('user_name')
    return render_template('yield.html',user_name=user_name)

if __name__ == '__main__':
    app.run(debug=True)
