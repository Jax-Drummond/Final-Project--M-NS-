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
        }
      } else
      {
        statusP.innerHTML = "❌ Invalid input.";
      }
    });

    document.getElementById("commandForm").addEventListener("submit", async function (e) {
      e.preventDefault();
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
        statusP.innerHTML = "✅ Drive command successful.";
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
        }
    };

    window.requestTelemetry = async function () {
      const statusP = document.getElementById("commandStatus");
      let endpoint = "/telementry_request";
      statusP.innerHTML = "ℹ️ Sending sleep command...";
      const response = await fetch(endpoint, {
        method: "GET"
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
        statusP.innerHTML = "✅ Sleep command successful.";
        }

    };
  });