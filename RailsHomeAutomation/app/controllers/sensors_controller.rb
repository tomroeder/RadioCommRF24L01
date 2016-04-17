class SensorsController < ApplicationController

  # Iterate through all elements in db an convert to string in format of javascript array [ [time1, value1], [time2, value2], time3, value3] ]
  # this format is understood by highcharts.
  def self.get_temperature()
    s = "[ "
    Sensor.all.each do |w|
      s = s + " [" + w.timestamp.to_datetime.strftime('%Q').to_s +  ", " + w.temperature.to_s + "] ,"
    end
    s = s + " ]"
    return s
  end

  def self.get_last_temperature()
    return Sensor.last.temperature.to_s
  end

  def self.get_last_time()
    return Sensor.last.timestamp.to_datetime.strftime('%Q').to_s
  end

  def self.get_last_pressure()
    return Sensor.last.pressure.to_s
  end

  # Iterate through all elements in db an convert to string in format of javascript array [ [time1, value1], [time2, value2], time3, value3] ]
  # this format is understood by highcharts.
  def self.get_pressure()
    s = "[ "
    Sensor.all.each do |w|
      s = s + " [" + w.timestamp.to_datetime.strftime('%Q').to_s +  ", " + w.pressure.to_s + "] ,"
    end
    s = s + " ]"
    return s
  end

end
