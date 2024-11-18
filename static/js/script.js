// Fetch pump frequency data from the server
async function fetchPumpFrequency() {
    try {
        const response = await fetch('/fetch_pump_frequency');
        if (response.ok) {
            const frequencyData = await response.json();
            renderScatterPlot(frequencyData); // Render the chart
        } else {
            console.error("Failed to fetch pump frequency data.");
        }
    } catch (error) {
        console.error("Error fetching pump frequency:", error);
    }
}

// Function to render the scatter plot using Chart.js
function renderScatterPlot(data) {
    const ctx = document.getElementById('scatterPlot').getContext('2d');
    new Chart(ctx, {
        type: 'line',  // Change to 'line' to connect the dots
        data: {
            datasets: [{
                label: 'Pump Frequency',
                data: data,  // Array of {x: date, y: frequency}
                backgroundColor: 'rgba(0, 123, 255, 0.5)',
                borderColor: 'rgba(0, 123, 255, 1)',
                borderWidth: 1,
                fill: false,  // Don't fill the area under the line
                tension: 0.1  // Smooth the curve if you want to
            }]
        },
        options: {
            responsive: true,  // Make the chart responsive
            maintainAspectRatio: false,  // Allow resizing without maintaining aspect ratio
            scales: {
                x: {
                    type: 'category',  // Use category scale for dates
                    title: {
                        display: true,
                        text: 'Date'
                    },
                    grid: {
                        display: false  // Remove grid on x-axis
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: 'Frequency'
                    },
                    min: 0,  // Start Y-axis from 0
                    grid: {
                        display: false  // Remove grid on y-axis
                    }
                }
            },
            plugins: {
                legend: {
                    display: false,  // Show legend
                },
            },
        }
    });
}

// Initialize all charts
function initCharts() {
    // Fetch pump frequency data and render scatter plot
    fetchPumpFrequency();
}

// Run on page load
window.onload = initCharts;
