#!/usr/bin/env python3
"""
Load secrets from firmware/secrets.env and inject as build flags
"""

Import("env")
import os

# Path to secrets.env file
secrets_path = os.path.join(env.get("PROJECT_DIR"), "firmware", "secrets.env")

# Default values if secrets.env doesn't exist
default_secrets = {
    "WIFI_SSID": "",
    "WIFI_PASSWORD": "",
    "WEATHER_API_KEY": "",
    "WEATHER_LOCATION": "Delhi"
}

secrets = default_secrets.copy()

# Load secrets if file exists
if os.path.exists(secrets_path):
    print(f"Loading secrets from: {secrets_path}")
    with open(secrets_path, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                key, value = line.split('=', 1)
                key = key.strip()
                value = value.strip().strip('"').strip("'")
                secrets[key] = value
    print(f"Loaded secrets: WIFI_SSID={secrets['WIFI_SSID'][:3]}***, WEATHER_LOCATION={secrets['WEATHER_LOCATION']}")
else:
    print(f"Warning: {secrets_path} not found, using empty defaults")

# Add build flags
build_flags = [
    f'-DDEFAULT_WIFI_SSID=\\"{secrets["WIFI_SSID"]}\\"',
    f'-DDEFAULT_WIFI_PASSWORD=\\"{secrets["WIFI_PASSWORD"]}\\"',
    f'-DDEFAULT_WEATHER_API_KEY=\\"{secrets["WEATHER_API_KEY"]}\\"',
    f'-DDEFAULT_WEATHER_LOCATION=\\"{secrets["WEATHER_LOCATION"]}\\"'
]

env.Append(CPPDEFINES=build_flags)
print(f"Injected {len(build_flags)} build flags from secrets.env")
