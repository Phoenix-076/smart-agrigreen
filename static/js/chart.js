ZC.LICENSE = ["569d52cefae586f634c54f86dc99e6a9", "b55b025e438fa8a98e32482b5f768ff5"];

const moistureConfig = {
  type: "gauge",
  globals: { 
    fontSize: 10  ,
    fontColor: '#ffff', 
    },
  plotarea: { marginTop: 60 },
  plot: {
    size: '100%',
    valueBox: {
      placement: 'center',
      text: '%v',
      fontSize: 14,
      rules: [
        { rule: '%v >= 3000', text: '%v<br>Too Dry' },
        { rule: '%v < 3000 && %v >= 2000', text: '%v<br>Good' },
        { rule: '%v < 2000', text: '%v<br>Too Wet' }
      ]
    }
  },
  scaleR: {
    aperture: 180,
    minValue: 0,
    maxValue: 5000,
    step: 1000,
    center: { visible: false },
    tick: { visible: false },
    labels: ['0', '1000', '2000', '3000', '4000', '5000'],
    ring: {
      size: 50,
      rules: [
        { rule: '%v < 2000', backgroundColor: 'yellow' },
        { rule: '%v >= 2000 && %v < 3000', backgroundColor: 'green' },
        { rule: '%v >= 3000', backgroundColor: 'red' }
      ]
    }
  },
  series: [{
    values: [0], // Default initial value
    indicator: [3, 3, 3, 3, 0.7], // Adjust pointer thickness here
    animation: { effect: 2, method: 1, sequence: 4, speed: 900 }
  }]
};

const moistureConfig2 = { 
  type: "gauge",
  globals: { 
    fontSize: 10  ,
    fontColor: '#ffff', 
    },
  plotarea: { marginTop: 60 },
  plot: {
    size: '100%',
    valueBox: {
      placement: 'center',
      text: '%v',
      fontSize: 14,
      rules: [
        { rule: '%v >= 3000', text: '%v<br>Too Dry' },
        { rule: '%v < 3000 && %v >= 2000', text: '%v<br>Good' },
        { rule: '%v < 2000', text: '%v<br>Too Wet' }
      ]
    }
  },
  scaleR: {
    aperture: 180,
    minValue: 0,
    maxValue: 5000,
    step: 1000,
    center: { visible: false },
    tick: { visible: false },
    labels: ['0', '1000', '2000', '3000', '4000', '5000'],
    ring: {
      size: 50,
      rules: [
        { rule: '%v < 2000', backgroundColor: 'yellow' },
        { rule: '%v >= 2000 && %v < 3000', backgroundColor: 'green' },
        { rule: '%v >= 3000', backgroundColor: 'red' }
      ]
    }
  },
  series: [{
    values: [0], // Default initial value
    indicator: [3, 3, 3, 3, 0.7], // Adjust pointer thickness here
    animation: { effect: 2, method: 1, sequence: 4, speed: 900 }
  }]
 };

const temperatureConfig = {
  type: "gauge",
  globals: { 
    fontSize: 10  ,
    fontColor: '#ffff', 
    },
  plotarea: { marginTop: 60 },
  plot: {
    size: '100%',
    valueBox: {
      placement: 'center',
      text: '%v',
      fontSize: 14,
      rules: [
        { rule: '%v <= 21', text: '%v<br>Too Cold' },
        { rule: '%v > 21 && %v <= 29', text: '%v<br>Good' },
        { rule: '%v > 29', text: '%v<br>Too Hot' }
      ]
    }
  },
  scaleR: {
    aperture: 180,
    minValue: 0,
    maxValue: 50,
    step: 10,
    center: { visible: false },
    tick: { visible: false },
    labels: ['0', '10', '20', '30', '40', '50'],
    ring: {
      size: 50,
      rules: [
        { rule: '%v < 20', backgroundColor: 'yellow' },
        { rule: '%v >= 20 && %v < 30', backgroundColor: 'green' },
        { rule: '%v >= 30', backgroundColor: 'red' }
      ]
    }
  },
  series: [{
    values: [0], // Default initial value
    indicator: [3, 3, 3, 3, 0.7], // Adjust pointer thickness here
    animation: { effect: 2, method: 1, sequence: 4, speed: 900 }
  }]
};

function updateMoistureGauge() {
  fetch('/get_soil_moisture')
    .then(response => response.json())
    .then(data => {
      const moistureValue = data.moisture1;
      moistureConfig.series[0].values = [moistureValue];
      zingchart.exec('gaugeMoisture', 'setseriesvalues', { values: [[moistureValue]] });
    });
}

function updateMoistureGauge2() {
    fetch('/get_soil_moisture')
    .then(response => response.json())
    .then(data => {
      const moistureValue = data.moisture2;
      moistureConfig.series[0].values = [moistureValue];
      zingchart.exec('gaugeMoisture2', 'setseriesvalues', { values: [[moistureValue]] });
    });
}
function updateTemperatureGauge() {
    fetch('/get_soil_moisture')
    .then(response => response.json())
    .then(data => {
      const temperatureValue = data.temperature;
      moistureConfig.series[0].values = [temperatureValue];
      zingchart.exec('gaugeTemperature', 'setseriesvalues', { values: [[temperatureValue]] });
    });
}

// Render all gauges in a single row
zingchart.render({ id: 'gaugeMoisture', data: moistureConfig, height: 300, width: '100%' });
zingchart.render({ id: 'gaugeMoisture2', data: moistureConfig2, height: 300, width: '100%' });
zingchart.render({ id: 'gaugeTemperature', data: temperatureConfig, height: 300, width: '100%' });

setInterval(updateMoistureGauge, 5000);
setInterval(updateMoistureGauge2, 5000);
setInterval(updateTemperatureGauge, 5000);
