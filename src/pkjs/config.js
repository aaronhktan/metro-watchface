module.exports = [
  { 
    "type": "heading", 
    "defaultValue": "Configuration Page" 
  }, 
  { 
    "type": "text", 
    "defaultValue": "You are running version 0.5 of Metro Watchface." 
  },
	{
		"type": "section",
		"items": [
			{
				"type": "color",
				"messageKey": "lineColour",
				"defaultValue": "FFFF00",
				"label": "Line Colour",
				"sunlight": "false"
			}
		]
	},
	{
		"type": "submit",
		"defaultValue": "Save Settings"
	}
]