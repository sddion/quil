import timezoneData from './timezones.json';

export interface Timezone {
  value: string;
  label: string;
  offset: number;
  offsetStr: string;
}

export interface TimezoneData {
  regions: Record<string, Timezone[]>;
  all: string[];
  popular: Timezone[];
}

// Export the full data
export const TIMEZONE_DATA = timezoneData as TimezoneData;

// Export regions for easy access
export const TIMEZONE_REGIONS = Object.keys(TIMEZONE_DATA.regions);

// Export flat list of all timezone values (IANA identifiers)
export const ALL_TIMEZONES = TIMEZONE_DATA.all;

// Export popular timezones from JSON
export const POPULAR_TIMEZONES = TIMEZONE_DATA.popular;

// Get timezones by region
export function getTimezonesByRegion(region: string): Timezone[] {
  return TIMEZONE_DATA.regions[region] || [];
}

// Get timezone display label with offset
export function getTimezoneLabel(value: string): string {
  for (const region of Object.values(TIMEZONE_DATA.regions)) {
    const tz = region.find(t => t.value === value);
    if (tz) {
      return `${tz.label} (${tz.offsetStr})`;
    }
  }
  return value;
}
