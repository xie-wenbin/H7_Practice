rd /Q /S MDK-ARM\Objects
rd /Q /S MDK-ARM\Listings
rd /Q /S MDK-ARM\DebugConfig
del /Q MDK-ARM\*.bak
del /Q MDK-ARM*.dep
del /Q MDK-ARM\JLink*
del /Q MDK-ARM\base.uvgui.*
del /Q MDK-ARM\base.uvguix.*

#del /Q project\EWARMv8\Project.dep
#del /Q project\EWARMv8\Debug
#del /Q project\EWARMv8\Flash
#del /Q project\EWARMv8\settings
#del /Q project\EWARMv8\Debug
#rd  /Q /S project\EWARMv8\Flash
#rd /Q /S project\EWARMv8\settings
#rd /Q /S project\EWARMv8\Debug
