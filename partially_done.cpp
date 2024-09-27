/*
----------------------Partially Done Codes------------------------------
 
  // for Saving the unlogged data when Wi-Fi is unavailable. When it becomes available again, all the saved data will be logged to the cloud
  {received_msg2 = Serial2.readString();
  received_msg2.trim();
  int id_received = received_msg2.toInt();
  x->ID.push_back(id_received);
  Serial2.print("received");

  x->count++;

  // Check if Wi-Fi is connected
  if (WiFi.status() == WL_CONNECTED) {
    // If there was previously offline data, send it first
    if (isOffline && !pending.animal_type.empty()) {
      for (int i = 0; i < pending.animal_type.size(); i++) {
        int type = pending.animal_type[i];
        int id = pending.ID[i];

        // Send offline data to Firebase
        switch (reg) {
          case SalesDelivery:
            if (type == 0) { // Pig
              json6.set(salesPig_ID.c_str(), String(id));
              json6.set(timePath6, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, salesPig_Path.c_str(), &json6);
            } else if (type == 1) { // Chicken
              json3.set(salesChic_ID.c_str(), String(id));
              json3.set(timePath3, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, salesChic_Path.c_str(), &json3);
            }
            break;

          case Recovery:
            if (type == 0) { // Pig
              json7.set(recoveryPig_ID.c_str(), String(id));
              json7.set(timePath7, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, recoveryPig_Path.c_str(), &json7);
            } else if (type == 1) { // Chicken
              json4.set(recoveryChic_ID.c_str(), String(id));
              json4.set(timePath4, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, recoveryChic_Path.c_str(), &json4);
            }
            break;

          case Deceased:
            if (type == 0) { // Pig
              json8.set(deceasedPig_ID.c_str(), String(id));
              json8.set(timePath8, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, deceasedPig_Path.c_str(), &json8);
            } else if (type == 1) { // Chicken
              json5.set(deceasedChic_ID.c_str(), String(id));
              json5.set(timePath5, "timestamp");
              Firebase.RTDB.pushJSON(&fbdo, deceasedChic_Path.c_str(), &json5);
            }
            break;
        }
      }

      // Clear pending data after successful sync
      pending.animal_type.clear();
      pending.ID.clear();
      pending.count = 0;
      isOffline = false;
    }

    // Send current data to Firebase
    switch (reg) {
      case SalesDelivery:
        if (animal_type_received == 0) { // Pig
          json6.set(salesPig_ID.c_str(), String(id_received));
          json6.set(timePath6, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, salesPig_Path.c_str(), &json6);
        } else if (animal_type_received == 1) { // Chicken
          json3.set(salesChic_ID.c_str(), String(id_received));
          json3.set(timePath3, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, salesChic_Path.c_str(), &json3);
        }
        break;

      case Recovery:
        if (animal_type_received == 0) { // Pig
          json7.set(recoveryPig_ID.c_str(), String(id_received));
          json7.set(timePath7, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, recoveryPig_Path.c_str(), &json7);
        } else if (animal_type_received == 1) { // Chicken
          json4.set(recoveryChic_ID.c_str(), String(id_received));
          json4.set(timePath4, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, recoveryChic_Path.c_str(), &json4);
        }
        break;

      case Deceased:
        if (animal_type_received == 0) { // Pig
          json8.set(deceasedPig_ID.c_str(), String(id_received));
          json8.set(timePath8, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, deceasedPig_Path.c_str(), &json8);
        } else if (animal_type_received == 1) { // Chicken
          json5.set(deceasedChic_ID.c_str(), String(id_received));
          json5.set(timePath5, "timestamp");
          Firebase.RTDB.pushJSON(&fbdo, deceasedChic_Path.c_str(), &json5);
        }
        break;
    }
  } else {
    // If offline, store data locally in the 'pending' struct
    pending.animal_type.push_back(animal_type_received);
    pending.ID.push_back(id_received);
    pending.count++;
    isOffline = true;
  }
}//registry update

// to automatic control the water and feed system 
void water_system_control(){
  if (waterLevel <= 30) // Check if water level is too low.
  {
    digitalWrite(water_control, HIGH); // Open the valve to fill water
    sendInt(outputPath + String(water_control), 1);   // Update output status to firebase.
  }
  if (waterLevel >= 60) // Check if water level is sufficient.
  {
    digitalWrite(water_control, LOW); // Close the valve to stop filling water
    sendInt(outputPath + String(water_control), 0);   // Update output status to firebase.
  }
}

#include "esp_sleep.h"

// for saving power consumption by entering sleep
void sleep(){
  // Configure the ESP32 to wake up on UART (serial input)
  esp_sleep_enable_uart_wakeup(USART_NUM_2); // wake when there is activity For Serial port 2
  esp_sleep_enable_uart_wakeup(USART_NUM_1); // wake when there is activity For Serial port 1
  esp_light_sleep_start();  // Enter light sleep 
}

// for exporting the table in Firebase to CSV format in js file
function exportTableToCSV() {
    var csv = 'Timestamp,Temperature,Humidity\n'; // CSV header

    // Read data from Firebase
    dbReadingsRef2.orderByKey().limitToLast(30).once('value', function(snapshot) {
        snapshot.forEach(function(childSnapshot) {
            var jsonData2 = childSnapshot.toJSON();
            var temperature2 = jsonData2.temperature2;
            var humidity2 = jsonData2.humidity2;
            var timestamp2 = jsonData2.timestamp;

            // Format the CSV row
            csv += epochToDateTime(timestamp2) + ',' + temperature2 + ',' + humidity2 + '\n';
        });

        // Create a Blob from the CSV string 
        var csvBlob = new Blob([csv], { type: 'text/csv' });

        // Create a download link
        var downloadLink = document.createElement('a');
        downloadLink.href = URL.createObjectURL(csvBlob);
        downloadLink.download = 'sensor2_data.csv';

        // Append the link to the body and trigger a click to download
        document.body.appendChild(downloadLink);
        downloadLink.click();

        // Clean up by removing the link
        document.body.removeChild(downloadLink);
    });
}

trigger this function with a button in HTML
<button onclick="exportTableToExcel()">Export as Excel</button>
*/
