
let isCooldown = 0;

let heartbeatInterval = null;

function startHeartbeat() {
  // Clear any existing interval to avoid duplicates
  if (heartbeatInterval) clearInterval(heartbeatInterval);

  heartbeatInterval = setInterval(async () => {
    console.log("🔄 Sending heartbeat...");
    try {
      const response = await fetch("/telementry_request", { method: "GET" });

      if (response.ok) {
        console.log("✅ Heartbeat succeeded");
      } else {
        console.warn("⚠️ Heartbeat failed with status:", response.status);
      }
    } catch (err) {
      console.error("❌ Heartbeat error:", err);
    }
  }, 60000); // every 30 seconds
}

function stopHeartbeat() {
  if (heartbeatInterval) {
    clearInterval(heartbeatInterval);
    heartbeatInterval = null;
    console.log("🛑 Heartbeat stopped");
  }
}

function startCooldown(ms) {
  isCooldown = true;
  setTimeout(() => {
    isCooldown = false;
    if(ms > 2000){
    document.getElementById("commandStatus").innerHTML = `Ready to send another command.`;
    }
  }, ms);
}

function wait(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

document.addEventListener("DOMContentLoaded", function () {
    document.getElementById("connectionForm").addEventListener("submit", async function (e) {
      e.preventDefault();
      const ip = document.getElementById("ip").value;
      const port = document.getElementById("port").value;
      const protocol = document.getElementById("protocol").value;
      const statusP = document.getElementById("connectionStatus");
      let endpoint = "/connect/";

      if (ip && port && protocol) {
        statusP.innerHTML = "ℹ️ Connecting to server please wait...";
        endpoint += ip + '/' + port + '/' + protocol;

        const response = await fetch(endpoint, {
            method: "POST"
        })

        if(response.status == 503)
        {
          statusP.innerHTML = "❌ Failed to connect to server.";
        }
        else if(response.status == 200)
        {
        statusP.innerHTML = "✅ Connected successfully.";
        await wait(1500);
        document.getElementById("connectionSection").classList.add("hidden");
        document.getElementById("commandSection").classList.remove("hidden");
        document.getElementById("commandSection").classList.add("slide-in");
        document.getElementById("commandStatus").innerHTML = "";

        startHeartbeat();
        }
      } else
      {
        statusP.innerHTML = "❌ Invalid input.";
      }
    });

    document.getElementById("commandForm").addEventListener("submit", async function (e) {
      e.preventDefault();

      if (isCooldown) return;

      const direction = document.getElementById("direction").value;
      const duration = document.getElementById("duration").value;
      const speed = document.getElementById("speed").value;
      const statusP = document.getElementById("commandStatus");
      let endpoint = "/telecommand";

      if (direction && duration && speed) {
        statusP.innerHTML = "ℹ️ Sending drive command...";
        endpoint += "?direction=" + direction + '&duration=' + duration + '&speed=' + speed;

        const response = await fetch(endpoint, {
            method: "PUT"
        })

        if(response.status == 503)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if (response.status == 400)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if(response.status == 200)
        {
        statusP.innerHTML = `✅ Drive command successful.</br>You will have to wait ${parseInt(duration) + 2}s before sending more commands.`;
        startCooldown((parseInt(duration) + 2) * 1000);
        }
      } else
      {
        statusP.innerHTML = "❌ Invalid input.";
      }
    });

    document.getElementById("toggleMode").addEventListener("click", function () {
        document.body.classList.toggle("dark");

        // Optional: store preference
        const mode = document.body.classList.contains("dark") ? "dark" : "light";
        localStorage.setItem("theme", mode);
      });

      // Load saved preference on load
      const savedTheme = localStorage.getItem("theme");
      if (savedTheme === "dark") {
        document.body.classList.add("dark");
      }

    window.sendSleep = async function () {
      if (isCooldown) return;
      const statusP = document.getElementById("commandStatus");
      let endpoint = "/telecommand?sleep=true";
        statusP.innerHTML = "ℹ️ Sending sleep command...";

        const response = await fetch(endpoint, {
            method: "PUT"
        })

        if(response.status == 503)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if (response.status == 400)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if(response.status == 200)
        {
        statusP.innerHTML = "✅ Sleep command successful.";
        await wait(2000);
        document.getElementById("connectionSection").classList.add("slide-in");
        document.getElementById("connectionSection").classList.remove("hidden");
        document.getElementById("commandSection").classList.remove("slide-in");
        document.getElementById("commandSection").classList.add("hidden");
        document.getElementById("connectionStatus").innerHTML = "";
        document.getElementById("commandForm").reset();
        document.getElementById("connectionForm").reset();
        }
        startCooldown(2000);
    };

    window.requestTelemetry = async function () {
      if (isCooldown) return;
      const statusP = document.getElementById("commandStatus");
      let endpoint = "/telementry_request";
      statusP.innerHTML = "ℹ️ Sending telem command...";
      const response = await fetch(endpoint, {
        method: "GET",
      });

      if(response.status == 503)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if (response.status == 400)
        {
          statusP.innerHTML = "❌ Failed to send packet.";
        }
        else if(response.status == 200)
        {
        statusP.innerHTML = "✅ Telemetry command successful.";
        startCooldown(2000);
        await wait(1000);
        response.json().then((data) => {
          console.log(data);
          statusP.innerHTML = `Last Packet Count: ${data.LPC}</br>Current Grade: ${data.currentGrade}</br>Hit Count: ${data.hitCount}</br>Last Cmd: ${data.lastCMD}</br>Last Cmd Speed: ${data.lastCMDSpeed}</br>Last Cmd Value: ${data.lastCMDValue}`;
        })
        }

    };
  });