from geopy.distance import distance #vincenty
from geopy.distance import geodesic #vincenty
tiananmen = (34.9073285, 108.391242416486)
xiaozhai = (34.9253171, 108.3426205)
print(geodesic(tiananmen, xiaozhai).meters)