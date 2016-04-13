class CreateSensors < ActiveRecord::Migration
  def change
    create_table :sensors do |t|
      t.datetime :timestamp
      t.float :temperature
      t.float :pressure

      t.timestamps
    end
  end
end
