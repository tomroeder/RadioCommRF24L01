class SensorsController < ApplicationController

  # Iterate through all elements in db an convert to string in format of javascript array [ [time1, value1], [time2, value2], time3, value3] ]
  # this format is understood by highcharts.
  # to_s - convert to string
  def self.get_temperature()
    s = "[ "
    Sensor.where(["timestamp > ?", (Time.now - 3.day)]).each do |w|
    # Sensor.all.each do |w|
      s = s + " [" + w.timestamp.to_datetime.strftime('%Q').to_s +  ", " + w.temperature.round(1).to_s + "] ,"
    end
    s = s + " ]"
    return s
  end

  # to_s - convert to string
  def self.get_last_temperature()
    return Sensor.last.temperature.round(1).to_s
  end

  # to_s - convert to string
  def self.get_last_time()
    return Sensor.last.timestamp.to_datetime.strftime('%Q').to_s
  end

  # to_s - convert to string
  def self.get_last_pressure()
    return Sensor.last.pressure.round(1).to_s
  end

  # return the pressure value in the middle between min and max. 
  def self.get_minmax_middle_pressure()
    #minPressure = Sensor.minimum("pressure")
    #maxPressure = Sensor.maximum("pressure")
    minPressure = Sensor.where(["timestamp>?", (Time.now-3.day)]).minimum("pressure")
    maxPressure = Sensor.where(["timestamp>?", (Time.now-3.day)]).maximum("pressure")
    d = maxPressure-minPressure
    return minPressure +d/2
  end

  # Iterate through all elements in db an convert to string in format of javascript array [ [time1, value1], [time2, value2], time3, value3] ]
  # this format is understood by highcharts.
  # round(1) - round to precision 1 after .
  # to_s - convert to string
  # Return only data from the last 3 days
  def self.get_pressure()
    s = "[ "
    Sensor.where(["timestamp > ?", (Time.now - 3.day)]).each do |w|
      # Sensor.where(timestamp: (Time.now.midnight - 2.day)..Time.now.midnight).each do |w|
      # Sensor.last(100).each do |w|
      # Sensor.all.each do |w|
      s = s + " [" + w.timestamp.to_datetime.strftime('%Q').to_s +  ", " + w.pressure.round(1).to_s + "] ,"
    end
     s = s + " ]"
    return s
  end

  # Iterate through all elements in db an convert to string in format of javascript array [ [time1, value1], [time2, value2], time3, value3] ]
  # this format is understood by highcharts.
  # round(1) - round to precision 1 after .
  # to_s - convert to string
  # Return only data from the last day
  def self.get_pressure_nowminus1day()
    s = "[ "
     Sensor.where(["timestamp > ?", (Time.now - 1.day)]).each do |w|
      s = s + " [" + w.timestamp.to_datetime.strftime('%Q').to_s +  ", " + w.pressure.round(1).to_s + "] ,"
    end
     s = s + " ]"
    return s
  end

 # return the pressure value in the middle between min and max. 
  def self.get_minmax_middle_pressure_nowminus1day()
    minPressure = Sensor.where(["timestamp>?", (Time.now-1.day)]).minimum("pressure")
    maxPressure = Sensor.where(["timestamp>?", (Time.now-1.day)]).maximum("pressure")
    d = maxPressure-minPressure
    return minPressure +d/2
  end
end
