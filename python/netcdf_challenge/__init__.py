from netCDF4 import Dataset
import pprint

rootgrp = Dataset("./concentration.timeseries.nc", "r", format="NETCDF4")

print("\n\nData Model\n")
print(rootgrp.data_model)
print("\n\nGroups\n")
print(rootgrp.groups)
print("\n\nDimensions\n")
pprint.pprint(rootgrp.dimensions)
print("\n\nVariables\n")
pprint.pprint(rootgrp.variables)

print("\n\nAttributes\n")
pprint.pprint(rootgrp.ncattrs)

rootgrp.close()
