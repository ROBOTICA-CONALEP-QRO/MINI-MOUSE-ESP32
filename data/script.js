function fetchMPUData() {
  fetch("/getMPUdata")
    .then((response) => response.json())
    .then((data) => {
      // Actualizar ángulos
      document.getElementById("angleX").textContent = data.angleX.toFixed(2);
      document.getElementById("angleY").textContent = data.angleY.toFixed(2);
      document.getElementById("angleZ").textContent = data.angleZ.toFixed(2);

      // Actualizar aceleración
      document.getElementById("ax").textContent = data.ax.toFixed(2);
      document.getElementById("ay").textContent = data.ay.toFixed(2);
      document.getElementById("az").textContent = data.az.toFixed(2);

      // Actualizar giroscopio
      document.getElementById("gx").textContent = data.gx.toFixed(2);
      document.getElementById("gy").textContent = data.gy.toFixed(2);
      document.getElementById("gz").textContent = data.gz.toFixed(2);

      // Actualizar temperatura
      document.getElementById("temperature").textContent =
        data.temperature.toFixed(1);

      // Actualizar offsets
      document.getElementById("offsetGyroX").textContent = data.offsets.gyroX;
      document.getElementById("offsetGyroY").textContent = data.offsets.gyroY;
      document.getElementById("offsetGyroZ").textContent = data.offsets.gyroZ;
      document.getElementById("offsetAccelX").textContent = data.offsets.accelX;
      document.getElementById("offsetAccelY").textContent = data.offsets.accelY;
      document.getElementById("offsetAccelZ").textContent = data.offsets.accelZ;
    })
    .catch((error) => {
      console.error("Error al obtener los datos del MPU:", error);
    });
}

function fetchSystemInfo() {
  fetch("/getSystemInfo")
    .then((response) => response.json())
    .then((data) => {
      // Actualizar información de memoria RAM
      document.getElementById("usedHeap").textContent = data.usedHeap;
      document.getElementById("totalHeap").textContent = data.totalHeap;
      const ramPercentage = (data.usedHeap / data.totalHeap) * 100;
      document.getElementById("ramProgress").style.width = ramPercentage + "%";

      // Actualizar información de Flash
      document.getElementById("usedFlash").textContent = data.usedFlash;
      document.getElementById("totalFlash").textContent = data.totalFlash;
      const flashPercentage = (data.usedFlash / data.totalFlash) * 100;
      document.getElementById("flashProgress").style.width =
        flashPercentage + "%";

      // Actualizar información del chip
      document.getElementById("chipModel").textContent = data.chipModel;
      document.getElementById("chipCores").textContent = data.chipCores;
      document.getElementById("cpuFreq").textContent = data.cpuFreqMHz;
    })
    .catch((error) => {
      console.error("Error al obtener la información del sistema:", error);
    });
}

// Función para obtener datos de los sensores de proximidad
function fetchProximityData() {
  fetch("/getProximityData")
    .then((response) => response.json())
    .then((data) => {
      // Actualizar distancias de los sensores
      document.getElementById("frontDistance").textContent = data.front;
      document.getElementById("leftDistance").textContent = data.left;
      document.getElementById("rightDistance").textContent = data.right;
    })
    .catch((error) => {
      console.error("Error al obtener los datos de proximidad:", error);
    });
}

// Nueva función para obtener el estado de detección de línea
function fetchLineDetection() {
  fetch("/getLineDetection")
    .then((response) => response.json())
    .then((data) => {
      const lineDetectedElement = document.getElementById("lineDetected");
      const lineDetectorSection = document.getElementById("lineDetectorSection");

      // Actualizar texto
      lineDetectedElement.textContent = data.lineDetected ? "SÍ" : "NO";

      // Actualizar estilo visual según detección
      if (data.lineDetected) {
        lineDetectorSection.classList.remove("no-line-detected");
        lineDetectorSection.classList.add("line-detected");
      } else {
        lineDetectorSection.classList.remove("line-detected");
        lineDetectorSection.classList.add("no-line-detected");
      }
    })
    .catch((error) => {
      console.error("Error al obtener los datos de detección de línea:", error);
    });
}

// Llama a fetchMPUData cada 500 ms (0.5 segundos)
setInterval(fetchMPUData, 500);

// Llama a fetchSystemInfo cada 3000 ms (3 segundos)
setInterval(fetchSystemInfo, 3000);

// Llama a fetchProximityData cada 500 ms para lecturas frecuentes
setInterval(fetchProximityData, 500);

// Llama a fetchLineDetection cada 200 ms para obtener lecturas más rápidas del detector de línea
setInterval(fetchLineDetection, 200);

// Llama a todas las funciones inmediatamente al cargar la página
fetchMPUData();
fetchSystemInfo();
fetchProximityData();
fetchLineDetection();
