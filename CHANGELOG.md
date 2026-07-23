# Changelog

All notable changes to TSMS projects will be documented in this file.

## [0.0.1.7] - 2024-09-09 TSMS_Master

### FIXED

- GetCurrentDateTime() function in the DateTime.h was not culture independent and does not work correctly with Windows' Hungarian culture settings.
  "%d.%m.%Y %H:%M:%S" format string has changed to this universal one: "%Y-%m-%d %H:%M:%S"

- ResultBuffer::PartOK function returned True value if only one measured value was good since on the TSMS_Vis's UI showed wrong good part counter value.

### ADDED

- PAT narrowing function for all measurable parameters

## [0.0.1.7] - 2024-09-09 TSMS_Vis

### FIXED

- Adding missing translations to Resource.de.resx and  Resource.hu.resx files.

- TSMS_Style.xaml: LED control templates replaced with LED styles because the user could change the state of the LEDs on the UI. 
  In the style, the user activity is disabled with the IsEnabled property.

- Deleting the empty and unnecessary Resource.de.Designer.cs and Resource.hu.Designer.cs folders from \Resources folder because of the following error:
  Error	CS1504	Source file 'C:\temp\TSMPS Test\export\TSMS_Vis\Resources\Resource.de.Designer.cs' could not be opened -- Access to the path 'C:\temp\TSMPS Test\export\TSMS_Vis\Resources\Resource.de.Designer.cs' is denied.

  ### ADDED
  
- Adding function that always checks and automatically creates the export directories for pdf, wodi and csv files.

- Everything is translated to Hungarian and almost everything to German.

- Extended lot query/filter function in the PrintReportDialog


## [0.0.1.9] - 2024-11-20 TSMS_Master  (Carsten)
- Adding compiler switch to deactivate the C measuring device (see const.h line 5)
- Adapt dummy test to run without C measuring

## [0.0.1.9] - 2024-11-20 TSMS_Vis  (Carsten)
- Removing fixed indices in Report and Wodi-Export (check in a loop if value is available)
