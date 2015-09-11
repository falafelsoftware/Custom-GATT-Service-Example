using System;
using System.Linq;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Storage.Streams;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;

namespace HelloBLE
{
    public sealed partial class MainPage : Page
    {
        
        private GattCharacteristic _notifyCharacteristic;
        private GattCharacteristic _writeCharacteristic;
        private GattCharacteristic _readCharacteristic;
       
        public MainPage()
        {
            this.InitializeComponent();
            btnBlue.IsEnabled   = false;
            btnGreen.IsEnabled  = false;
            btnYellow.IsEnabled = false;
            btnOrange.IsEnabled = false;
            btnPurple.IsEnabled = false;
            btnRead.IsEnabled = false;
            SetupBLE();
        }

        public async void SetupBLE()
        {
            txtProgress.Text = "Obtaining BTLE Info...";
            var query = BluetoothLEDevice.GetDeviceSelector();
            var deviceList = await DeviceInformation.FindAllAsync(query);
            int count = deviceList.Count();
        
            if(count > 0)
            {
                //Assumes default name of the Adafruit Bluefruit LE
                var deviceInfo = deviceList.Where(x => x.Name == "Adafruit Bluefruit LE").FirstOrDefault();
                if(deviceInfo!=null)
                {
                    var bleDevice = await BluetoothLEDevice.FromIdAsync(deviceInfo.Id);
                    var deviceServices = bleDevice.GattServices;

                    txtProgress.Text = "Retrieving service and GATT characteristics...";
                    var deviceSvc = deviceServices.Where(svc => svc.AttributeHandle == 0x003a).FirstOrDefault();
                    if (deviceSvc != null)
                    {
                        var characteristics = deviceSvc.GetAllCharacteristics();
                        _notifyCharacteristic = characteristics.Where(x => x.AttributeHandle == 0x003b).FirstOrDefault();
                        _writeCharacteristic = characteristics.Where(x => x.AttributeHandle == 0x003e).FirstOrDefault();
                        _readCharacteristic = characteristics.Where(x => x.AttributeHandle == 0x0040).FirstOrDefault();
                        _notifyCharacteristic.ValueChanged += NotifyCharacteristic_ValueChanged;
                        await _notifyCharacteristic.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify);

                        txtProgress.Text = "Bluetooth LE Device service and characteristics initialized";
                        txtBTLEStatus.Text = "Initialized";
                        txtBTLEStatus.Foreground = new SolidColorBrush(Colors.Green);
                        btnBlue.IsEnabled   = true;
                        btnGreen.IsEnabled  = true;
                        btnYellow.IsEnabled = true;
                        btnOrange.IsEnabled = true;
                        btnPurple.IsEnabled = true;
                        btnRead.IsEnabled = true;
                    }
                    else
                    {
                        txtInfo.Text = "Custom GATT Service Not Found on the Bluefruit";
                    }
                }
                else
                {
                    txtInfo.Text = "Adafruit Bluefruit LE not found, is it paired ??";
                }
            } 
        }

        private void NotifyCharacteristic_ValueChanged(GattCharacteristic sender, 
            GattValueChangedEventArgs args)
        {
            //notification that the NeoPixel color has changed, update the UI with the new value
            byte[] bArray = new byte[args.CharacteristicValue.Length];
            DataReader.FromBuffer(args.CharacteristicValue).ReadBytes(bArray);

            var color = Color.FromArgb(0,bArray[0], bArray[1], bArray[2]);
            string result = color.ToString();

            //remove alpha channel from string (only rgb was returned)
            result = result.Remove(1, 2);

            Dispatcher.RunAsync(CoreDispatcherPriority.Low, () => { txtLastNotificationHex.Text = result; });
            
        }

        
        private async void btnColor_Click(object sender, RoutedEventArgs e)
        {
            //Change the color of the NeoPixel by writing the hex color bytes to the Write characteristic
            //that is currently being monitored by our Flora sketch
            Button btnColor = (Button)sender;
            var color = ((SolidColorBrush)btnColor.Background).Color;

            var writer = new Windows.Storage.Streams.DataWriter();
            writer.WriteBytes(new byte[] { color.R, color.G, color.B });

            txtProgress.Text = "Writing color to Writable GATT characteristic ...";
            await _writeCharacteristic.WriteValueAsync(writer.DetachBuffer());
            
            txtProgress.Text = "Writable GATT characteristic written";

        }

        private async void btnRead_Click(object sender, RoutedEventArgs e)
        {
            // Read the current color of the NeoPixel by querying the Read characteristic
            txtProgress.Text = "Reading GATT characteristic";
            var result = await _readCharacteristic.ReadValueAsync(BluetoothCacheMode.Uncached);
            if(result.Status== GattCommunicationStatus.Success)
            {
                txtProgress.Text = "Characteristic has been read";
               
                byte[] bArray = new byte[result.Value.Length];
                DataReader.FromBuffer(result.Value).ReadBytes(bArray);

                var color = Color.FromArgb(0, bArray[0], bArray[1], bArray[2]);
                string colorString = color.ToString();
                //remove alpha channel from string (only rgb was returned)
                colorString = colorString.Remove(1, 2);
                txtReadValue.Text = colorString;
            }
            else
            {
                txtProgress.Text = "ERROR";
                txtInfo.Text = "There was a problem reading the GATT characteristic";
            }
        }
    }
}
