#!/usr/bin/env node
import { join } from 'path';
import { readFileSync, writeFileSync } from 'fs';
import * as readline from 'readline';

// App paths
const packageJsonPath = join(process.cwd(), 'app/package.json');
const appJsonPath = join(process.cwd(), 'app/app.json');
const docsHtmlPath = join(process.cwd(), 'docs/index.html');

// Server path
const denoJsonPath = join(process.cwd(), 'server/deno.json');

// Firmware path
const firmwareConfigPath = join(process.cwd(), 'firmware/include/config.h');

// Central version file
const versionJsonPath = join(process.cwd(), 'version.json');

const packageJson = JSON.parse(readFileSync(packageJsonPath, 'utf-8'));
const appJson = JSON.parse(readFileSync(appJsonPath, 'utf-8'));

const currentVersion = packageJson.version;

console.log(`Current version: ${currentVersion}`);

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

rl.question('Do you want to bump the version? (y/N) ', (answer) => {
    if (answer.toLowerCase() === 'y') {
        rl.question('Which version to bump? (1: Patch [default], 2: Minor, 3: Major) ', (type) => {
            const parts = currentVersion.split('.').map(Number);

            switch (type.trim()) {
                case '2':
                    parts[1] += 1;
                    parts[2] = 0;
                    break;
                case '3':
                    parts[0] += 1;
                    parts[1] = 0;
                    parts[2] = 0;
                    break;
                default:
                    parts[2] += 1;
                    break;
            }

            const newVersion = parts.join('.');
            const today = new Date().toISOString().split('T')[0];

            // Update central version.json
            const versionJson = { version: newVersion, description: "Centralized version for Quil", updated: today };
            writeFileSync(versionJsonPath, JSON.stringify(versionJson, null, 2) + '\n');

            // Update app package.json and app.json
            packageJson.version = newVersion;
            appJson.expo.version = newVersion;
            writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2) + '\n');
            writeFileSync(appJsonPath, JSON.stringify(appJson, null, 2) + '\n');

            // Update server deno.json
            try {
                const denoJson = JSON.parse(readFileSync(denoJsonPath, 'utf-8'));
                denoJson.version = newVersion;
                writeFileSync(denoJsonPath, JSON.stringify(denoJson, null, 4) + '\n');
                console.log('  - server/deno.json');
            } catch (e) {
                console.warn('  - server/deno.json (Failed to update)');
            }

            // Update firmware config.h
            try {
                let configContent = readFileSync(firmwareConfigPath, 'utf-8');
                const versionRegex = /#define FIRMWARE_VERSION "[\d\.]+"/;
                if (versionRegex.test(configContent)) {
                    configContent = configContent.replace(versionRegex, `#define FIRMWARE_VERSION "${newVersion}"`);
                    writeFileSync(firmwareConfigPath, configContent, 'utf-8');
                    console.log('  - firmware/include/config.h');
                }
            } catch (e) {
                console.warn('  - firmware/include/config.h (Failed to update)');
            }


            console.log(`\nVersion bumped to ${newVersion}`);
            console.log('\nUpdated files:');
            console.log('  - app/package.json');
            console.log('  - app/app.json');
            console.log('  - server/deno.json');
            console.log('  - firmware/include/config.h');
            console.log('\nNote: Version is also displayed in:');
            console.log('  - App UI (via Constants.expoConfig.version)');
            console.log('  - BLE status (firmwareVersion in DeviceStatus)');

            const { execSync } = require('child_process');
            try {
                execSync(`git add version.json app/package.json app/app.json server/deno.json firmware/include/config.h docs/index.html 2>/dev/null || true`);
                console.log('\nFiles staged for commit.');
            } catch (e) {
                console.error("\nFailed to add updated version files to git");
            }
            rl.close();
            process.exit(0);
        });
    } else {
        console.log('Version not changed.');
        rl.close();
        process.exit(0);
    }
});
