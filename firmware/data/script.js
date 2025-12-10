document.addEventListener('DOMContentLoaded', function() {
  const form = document.getElementById('wifi-form');
  const ssidSelect = document.getElementById('ssid');
  const passwordInput = document.getElementById('password');
  const togglePassBtn = document.getElementById('toggle-pass');
  const refreshBtn = document.getElementById('refresh-btn');
  const connectBtn = document.getElementById('connect-btn');
  const btnText = document.getElementById('btn-text');
  const btnLoader = document.getElementById('btn-loader');
  const statusDiv = document.getElementById('status');
  const successDiv = document.getElementById('success');
  const deviceIpSpan = document.getElementById('device-ip');

  // Scan for networks on load
  scanNetworks();

  // Toggle password visibility
  togglePassBtn.addEventListener('click', function() {
    if (passwordInput.type === 'password') {
      passwordInput.type = 'text';
      togglePassBtn.textContent = 'Hide';
    } else {
      passwordInput.type = 'password';
      togglePassBtn.textContent = 'Show';
    }
  });

  // Refresh networks
  refreshBtn.addEventListener('click', function() {
    scanNetworks();
  });

  // Form submission
  form.addEventListener('submit', async function(e) {
    e.preventDefault();
    
    const ssid = ssidSelect.value;
    const password = passwordInput.value;
    
    if (!ssid) {
      showStatus('Please select a WiFi network', 'error');
      return;
    }
    
    setLoading(true);
    hideStatus();
    
    try {
      const response = await fetch('/api/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid, password })
      });
      
      const data = await response.json();
      
      if (data.success) {
        // Poll for connection status
        pollConnectionStatus();
      } else {
        showStatus(data.message || 'Connection failed', 'error');
        setLoading(false);
      }
    } catch (err) {
      showStatus('Failed to connect. Please try again.', 'error');
      setLoading(false);
    }
  });

  async function scanNetworks() {
    ssidSelect.innerHTML = '<option value="">Scanning...</option>';
    ssidSelect.disabled = true;
    refreshBtn.disabled = true;
    
    let attempts = 0;
    const maxAttempts = 15; // 30 seconds max (2s interval)

    while (attempts < maxAttempts) {
        try {
            const response = await fetch('/api/scan');
            const data = await response.json();
            
            if (data.networks && data.networks.length > 0) {
                // Success - Populate List
                ssidSelect.innerHTML = '<option value="">Select a network</option>';
                
                // Sort by signal
                data.networks.sort((a, b) => b.rssi - a.rssi);
                
                data.networks.forEach(network => {
                    const option = document.createElement('option');
                    option.value = network.ssid;
                    option.textContent = network.ssid + (network.rssi ? ` (${network.rssi}dBm)` : '');
                    ssidSelect.appendChild(option);
                });
                
                ssidSelect.disabled = false;
                refreshBtn.disabled = false;
                return; // Done
            }
            
            // Empty list (Async scan running), wait and retry
            attempts++;
            await new Promise(r => setTimeout(r, 2000));
            
        } catch (err) {
            // Network error, likely retry
            attempts++;
            await new Promise(r => setTimeout(r, 2000));
        }
    }
    
    // Timeout
    ssidSelect.innerHTML = '<option value="">No networks found (Retrying...)</option>';
    ssidSelect.disabled = false;
    refreshBtn.disabled = false;
  }

  async function pollConnectionStatus() {
    let attempts = 0;
    const maxAttempts = 30; // 30 seconds timeout
    
    const poll = async () => {
      try {
        const response = await fetch('/api/status');
        const data = await response.json();
        
        if (data.connected) {
          showSuccess(data.ip);
          return;
        }
        
        if (data.failed) {
          showStatus('Connection failed. Check password and try again.', 'error');
          setLoading(false);
          return;
        }
        
        attempts++;
        if (attempts < maxAttempts) {
          setTimeout(poll, 1000);
        } else {
          showStatus('Connection timeout. Please try again.', 'error');
          setLoading(false);
        }
      } catch (err) {
        showStatus('Lost connection to device.', 'error');
        setLoading(false);
      }
    };
    
    poll();
  }

  function showSuccess(ip) {
    form.classList.add('hidden');
    successDiv.classList.remove('hidden');
    deviceIpSpan.textContent = ip;
    setLoading(false);
  }

  function showStatus(message, type) {
    statusDiv.textContent = message;
    statusDiv.className = 'status ' + type;
    statusDiv.classList.remove('hidden');
  }

  function hideStatus() {
    statusDiv.classList.add('hidden');
  }

  function setLoading(loading) {
    connectBtn.disabled = loading;
    ssidSelect.disabled = loading;
    passwordInput.disabled = loading;
    refreshBtn.disabled = loading;
    
    if (loading) {
      btnText.textContent = 'Connecting...';
      btnLoader.classList.remove('hidden');
    } else {
      btnText.textContent = 'Connect';
      btnLoader.classList.add('hidden');
    }
  }

});
