import 'dart:async';
import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:http/http.dart' as http;

import 'chart.dart';

import 'dart:math' as math;

//import 'package:charts_flutter/flutter.dart' as charts;

void main() {
  runApp(new MaterialApp(
    theme: new ThemeData(
      primarySwatch: Colors.green,
    ),
    home: new MyApp(),
    routes: <String, WidgetBuilder>{
      "/Charts": (BuildContext context) =>
          new Charts(_MyAppState._createRandomData(), true),
      //add more routes here
    },
  ));
}

///
///
///
class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => new _MyAppState();
}

enum Status { search, connect, disconnect }
enum BTstatus { start, mode0, mode1, send }

///
///
///
class _MyAppState extends State<MyApp> {
  FlutterBluetoothSerial bluetooth = FlutterBluetoothSerial.instance;

  List<BluetoothDevice> _devices = [];
  BluetoothDevice _device;
  int _state = -7;

  static final TextEditingController _btResponse = new TextEditingController();
  BTstatus _btStatus = BTstatus.start;
  List<String> _ghDatas = new List();
  List<String> _ghDatasOnline = new List();
  var _lastDatas = <Widget>[];

  Status _stat = Status.search;
  bool _isOnline = false;
  Sera _lastData0 = new Sera(0, 22.2, 34.4, 5),
      _lastData1 = new Sera(1, 22.2, 34.4, 5),
      _lastData2 = new Sera(2, 22.2, 34.4, 5);

  ///
  ///
  ///
  @override
  void initState() {
    super.initState();

    _isOnline = false;

    bluetooth.onStateChanged().listen((state) {
      switch (state) {
        case FlutterBluetoothSerial.CONNECTED:
          setState(() {
            // _isConnected = true;
            //_pressed = false;
          });
          break;
        case FlutterBluetoothSerial.DISCONNECTED:
          setState(() {
            // _isConnected = false;
            //_pressed = false;
          });
          break;
        case FlutterBluetoothSerial.STATE_OFF:
          setState(() {
            //  _isConnected = false;
            //_pressed = false;
            _stat = Status.search;
            _devices.clear();
            _device = null;
          });
          break;
        default:
          // TODO
          print(state);
          break;
      }
      setState(() {
        _state = state;
        print('state changed' + _state.toString());
      });
    });

    bluetooth.onRead().listen((msg) {
      setState(() {
        if (_btStatus == BTstatus.start) {
          _btResponse.text += msg;
          if (_btResponse.text.contains("\r\n")) {
            //_btResponse.text += msg;
            if (_btResponse.text.contains("0\r\n")) {
              _isOnline = false;
              //print("anan");
            } else if (_btResponse.text.contains("1\r\n")) {
              _isOnline = true;
              //print("anan");
            } else {
              _showAlert("Yanlış cihaza bağlısınız.");
              _disconnect();
            }
            _btResponse.clear();
          }
        } else if (_btStatus == BTstatus.send) {
          _btResponse.text += msg;
          if (_btResponse.text.contains("\r\n")) {
            //_btResponse.text += msg;
            _ghDatas.add(_btResponse.text);
            print(_btResponse.text);
            print(_ghDatas.length);
            _btResponse.clear();
          }
        } else if (_btStatus == BTstatus.mode0) {
          _btResponse.text += msg;
          if (_btResponse.text.contains("\r\n")) {
            //_btResponse.text += msg;
            if (!_btResponse.text.contains("mode change 0\r\n")) {
              _showAlert("Mod değiştirilemedi.");
              //_disconnect();
              //print("anan");
            }
            _isOnline = false;
            _btResponse.clear();
          }
        } else {
          _btResponse.text += msg;
          if (_btResponse.text.contains("\r\n")) {
            //_btResponse.text += msg;
            if (!_btResponse.text.contains("mode change 1\r\n")) {
              _showAlert("Mod değiştirilemedi.");
              //_disconnect();
              //print("anan");
            }
            _isOnline = true;
            _btResponse.clear();
          }
        }
      });
    });

    //initPlatformState();
  }


  Future _scanBTDevices() async {
    bool key = await bluetooth.isOn, key2 = !(await bluetooth.isConnected);

    if (key && key2) {
      List<BluetoothDevice> temp = await bluetooth.getBondedDevices();
      setState(() {
        _devices = temp;
        _stat = Status.connect;
        print('cihazlar bulundu!!');
        //_showAlert("Cihaz listede yoksa cihazınızı eşleştirin !");
      });
      _showSnackBar("Arama tamamlandı !");
    } else {
      if (!key) {
        _showAlert("Lütfen Bluetooth'u açınız");
      } else if (!key2) {
        _showAlert("Zaten Bağlısınız");
      }
    }
  }

  void _connect() async {
    if (_device == null) {
      _showAlert('Cihaz seçilmedi.');
    } else {
      bool key = await bluetooth.isOn && !(await bluetooth.isConnected);
      if (key) {
        bluetooth.connect(_device).then((val) {
          setState(() => {
                _stat = Status.disconnect,
                // _isConnected = true,
              });

          bluetooth.write("start").then((onValue) {
            setState(() {
              _btStatus = BTstatus.start;
            });
          });
          _showSnackBar("Bağlantı başarılı !");
        });
      }
    }
  }

  Future _disconnect() async {
    bool key = await bluetooth.isConnected;
    if (key) {
      bluetooth.disconnect().then((val) {
        setState(() => {_stat = Status.connect, _btResponse.clear()});
      });
    } else {
      bool key = await bluetooth.isOn;
      if (key) {
        setState(() => _stat = Status.connect);
      } else {
        setState(() => _stat = Status.search);
      }
    }
  }

  Future _getGHDatas() async {
    bool key = await bluetooth.isConnected;
    if (key) {
      bluetooth.write("send").then((onValue) {
        setState(() {
          _btStatus = BTstatus.send;
        });
      });
    } else {
      _showAlert("Bağlı değilsiniz.");
    }
  }


  Future<GHhttpDataList> _getGHDatasFromServer() async {
    final response = await http.get('http://23.97.238.10:5000');
    //await http.get('http://23.97.238.10:5000/?ghID=0');

    if (response.statusCode == 200) {
      // If server returns an OK response, parse the JSON
      GHhttpDataList temp = GHhttpDataList.fromJson(json.decode(response.body));

      int size = temp.ghDatas.length;
      bool key0 = false, key1 = false, key2 = false;
      String data;
      for (int i = 0; i < size; ++i) {
        data = temp.ghDatas[i].toString();
        _ghDatasOnline.insert(i, data);

        List<String> tempStr = data.split("&");
        if (!key0 && data[0] == "0") {
          setState(() {
            _lastData0 = new Sera(
                int.parse(tempStr[0]),
                double.parse(tempStr[1]).roundToDouble(),
                double.parse(tempStr[2]).roundToDouble(),
                int.parse(tempStr[3]));
          });
          key0 = true;
        } else if (!key1 && data[0] == "1") {
          setState(() {
            _lastData1 = new Sera(
                int.parse(tempStr[0]),
                double.parse(tempStr[1]).roundToDouble(),
                double.parse(tempStr[2]).roundToDouble(),
                int.parse(tempStr[3]));
          });
          key1 = true;
        } else if (!key2 && data[0] == "2") {
          setState(() {
            _lastData2 = new Sera(
                int.parse(tempStr[0]),
                double.parse(tempStr[1]),
                double.parse(tempStr[2]).roundToDouble(),
                int.parse(tempStr[3]));
          });
          key2 = true;
        }
        //print(temp.ghDatas[i].toString());
      }

      return temp;
    } else {
      // If that response was not OK, throw an error.
      //throw Exception('Failed to load post');
      _showAlert("Veriler alınamadı. İnternet bağlantısını kontrol ediniz.");
    }

    //await _getLastDatas();
  }

  Future _changeRunMode() async {
    bool key = await bluetooth.isConnected;
    if (key) {
      if (_isOnline) {
        // çalışmayabilir, test et,
        bluetooth.write("mode=1").then((onValue) {
          setState(() {
            _btStatus = BTstatus.mode1;
          });
        });
      } else {
        bluetooth.write("mode=0").then((onValue) {
          setState(() {
            _btStatus = BTstatus.mode0;
          });
        });
      }
    } else {
      _showAlert("Bağlı değilsiniz.");
      setState(() {
        //isOffline = !isOffline;
      });
    }
  }

  Widget _setBTButton() {
    if (_stat == Status.search) {
      return new FloatingActionButton(
          mini: true,
          child: new Icon(
            Icons.search,
            size: 20,
          ),
          onPressed: _scanBTDevices);
    } else if (_stat == Status.connect) {
      return new Row(
        children: <Widget>[
          new RaisedButton(child: new Text("Bağlan"), onPressed: _connect),
          new FloatingActionButton(
              mini: true,
              child: new Icon(
                Icons.search,
                size: 20,
              ),
              onPressed: _scanBTDevices)
        ],
      );
    } else {
      return new FloatingActionButton(
          mini: true,
          child: new Icon(
            Icons.stop,
            color: Colors.red,
            size: 20,
          ),
          onPressed: _disconnect);
    }
  }

  List<DropdownMenuItem<BluetoothDevice>> _getDeviceItems() {
    List<DropdownMenuItem<BluetoothDevice>> items = [];
    if (_devices.isEmpty) {
      items.add(DropdownMenuItem(
        child: new Container(
          child: Text("Bağlı Değil"),
          constraints: BoxConstraints(maxWidth: 150),
        ),
      ));
    } else {
      _devices.forEach((device) {
        items.add(DropdownMenuItem(
          child: Container(
            child: Text(device.name),
            constraints: BoxConstraints(maxWidth: 150),
          ),
          value: device,
        ));
      });
    }
    return items;
  }

  Widget _BTImage() {
    if (_stat == Status.search) {
      return new Icon(
        Icons.bluetooth,
        color: Colors.blueAccent,
        size: 30,
      );
    } else if (_stat == Status.connect) {
      return new Icon(
        Icons.bluetooth_searching,
        color: Colors.blueAccent,
        size: 30,
      );
    } else {
      return new Icon(
        Icons.bluetooth_connected,
        color: Colors.blueAccent,
        size: 30,
      );
    }
  }

  final GlobalKey<ScaffoldState> _scaffoldstate =
      new GlobalKey<ScaffoldState>();

  void _showSnackBar(String value) {
    if (value.isEmpty) return;
    _scaffoldstate.currentState.showSnackBar(
      new SnackBar(
        content: new Text(value),
        backgroundColor: Colors.green,
      ),
    );
  }

  void _showAlert(String value) {
    if (value.isEmpty) return;

    AlertDialog dialog = new AlertDialog(
      content: new Text(
        value,
        style: new TextStyle(fontSize: 20.0),
      ),
      actions: <Widget>[
        new Center(
          child: new FlatButton(
            onPressed: () {
              Navigator.of(context).pop();
            },
            child: new Text("Tamam"),
          ),
        )
      ],
    );

    showDialog(
        context: context,
        builder: (BuildContext) {
          return dialog;
        });
  }

  /// Create random data.
  static List<GreenHouseData> _createRandomData() {
    final random = new math.Random();

    List<GreenHouseData> temp = new List();
    for (int i = 0; i < 168; ++i) {
      temp.add(new GreenHouseData(
          0,
          new DateTime(2019, 5, i + 1),
          random.nextDouble() * 40,
          random.nextDouble() * 60,
          random.nextInt(100) % 5));
      temp.add(new GreenHouseData(
          1,
          new DateTime(2019, 5, i + 1),
          random.nextDouble() * 30,
          random.nextDouble() * 50,
          random.nextInt(100) % 5));
      temp.add(new GreenHouseData(
          2,
          new DateTime(2019, 5, i + 1),
          random.nextDouble() * 50,
          random.nextDouble() * 70,
          random.nextInt(100) % 5));
    }

    return temp;
  }

  @override
  Widget build(BuildContext context) {
    return new Scaffold(
        key: _scaffoldstate,
        appBar: new AppBar(
          title: const Text('Sera İzleme Sistemi'),
          //actions: _buildActionButtons(),
        ),
        //floatingActionButton: _buildScanningButton(),
        body: new Container(
            padding: EdgeInsets.only(top: 10),
            child: new Center(
                child: new ListView(
              //shrinkWrap: true,
              children: <Widget>[
                new Row(
                  //crossAxisAlignment: CrossAxisAlignment.center,
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: <Widget>[
                    // online , offline
                    _BTImage(),
                    (_stat == Status.disconnect)
                        ? new Container(
                            child: Text(_device.name),
                            constraints: BoxConstraints(maxWidth: 150),
                          )
                        : new DropdownButton(
                            //hint: new Text("Bağlı değil"),
                            items: _getDeviceItems(),
                            onChanged: (value) =>
                                setState(() => _device = value),
                            value: _device,
                            //disabledHint: Icon(Icons.bluetooth),
                            //icon: Icon(Icons.bluetooth),
                          ),
                    _setBTButton(),
                  ],
                ),
                Container(
                    padding: EdgeInsets.all(30),
                    child: Column(
                      children: <Widget>[
                        new Text(
                          "Çalışma Modu",
                          style: TextStyle(fontSize: 20),
                        ),
                        new Row(
                          mainAxisAlignment: MainAxisAlignment.center,
                          children: <Widget>[
                            Text(
                              "Local Mod",
                              style: TextStyle(fontSize: 20),
                            ),
                            Center(
                              child: new Switch(
                                  value: _isOnline,
                                  onChanged: (bool value) {
                                    setState(() {
                                      _isOnline = value;
                                      print(_isOnline);
                                    });
                                    _changeRunMode();
                                  }),
                            ),
                            Text(
                              "Online Mod",
                              style: TextStyle(fontSize: 20),
                            ),
                          ],
                        ),
                      ],
                    )),
                new Container(
                    padding:
                        const EdgeInsets.only(top: 20.0, left: 20, right: 20),
                    child: new Center(
                      child: new Column(
                        children: <Widget>[
                          new Text(
                            "Son Veriler",
                            style: TextStyle(
                              fontSize: 20,
                            ),
                          ),
                          new Column(
                            //crossAxisAlignment: CrossAxisAlignment.center,
                            mainAxisAlignment: MainAxisAlignment.center,
                            children: <Widget>[
                              Container(
                                padding: EdgeInsets.only(bottom: 10),
                              ),
                              _showDatas(),
                              Container(
                                padding: EdgeInsets.only(top: 10),
                                child: new RaisedButton(
                                    child: new Text(
                                      'Verileri Al',
                                      style: TextStyle(fontSize: 20),
                                    ),
                                    onPressed: () {
                                      (_isOnline == true)
                                          ? _getGHDatasFromServer()
                                          : _getGHDatas();
                                    }),
                              ),
                            ],
                          ),
                        ],
                      ),
                    )),
                Padding(
                  padding: EdgeInsets.only(
                      top: 100, left: 25, right: 25, bottom: 10),
                  child: new RaisedButton(
                      child: new Text(
                        'Grafikler',
                        style: TextStyle(fontSize: 20),
                      ),
                      onPressed: () {
                        Navigator.of(context).pushNamed('/Charts');
                      }),
                )
              ],
            ))));
  }


  _showDatas() {
    setState(() {
      _lastDatas.clear();
    });

    if (_isOnline) {
      if (_lastData0 != null) _lastDatas.add(_lastData0);

      if (_lastData1 != null) _lastDatas.add(_lastData1);

      if (_lastData2 != null) _lastDatas.add(_lastData2);
    } else {
      _lastDatas.add(_lastData0);
      _lastDatas.add(_lastData1);
      _lastDatas.add(_lastData2);
    }

    return new Column(
      children:
          _lastDatas, /* <Widget>[

          new Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: <Widget>[
              new Text(
                "Sera",
                style: TextStyle(fontSize: 20),
              ),
              Expanded(
                child: Container(
                  padding: EdgeInsets.only(right: 50),
                ),
              ),
              new Text(
                "Sıcaklık",
                style: TextStyle(fontSize: 20),
              ),
              Expanded(
                child: Container(
                  padding: EdgeInsets.only(right: 50),
                ),
              ),
              new Text(
                "Nem",
                style: TextStyle(fontSize: 20),
              ),
              Expanded(
                child: Container(
                  padding: EdgeInsets.only(right: 50),
                ),
              ),
              new Text(
                "Depo",
                style: TextStyle(fontSize: 20),
              ),
            ],
          ),



          new Sera(0, 22.2, 34.4, 5),
          new Sera(1, 22.2, 34.4, 5),
          new Sera(2, 22.2, 34.4, 5),
        ],
        */
    );
  }
}

class GHhttpDataList {
  final List<GHhttpData> ghDatas;

  GHhttpDataList({
    this.ghDatas,
  });

  factory GHhttpDataList.fromJson(List<dynamic> parsedJson) {
    List<GHhttpData> datas = new List<GHhttpData>();
    datas = parsedJson.map((i) => GHhttpData.fromJson(i)).toList();

    return new GHhttpDataList(ghDatas: datas);
  }
}

class GHhttpData {
  final int id;
  final double temp;
  final double humidity;
  final int level;

  GHhttpData({this.id, this.temp, this.humidity, this.level});

  @override
  String toString() {
    //return "?ghID="+id.toString()+ "&temp"+temp.toString()+ "&hum"+ humidity.toString()+"&level"+ level.toString();
    return id.toString() +
        "&" +
        temp.toString() +
        "&" +
        humidity.toString() +
        "&" +
        level.toString();
  }

  factory GHhttpData.fromJson(Map<String, dynamic> json) {
    return GHhttpData(
      id: json['ghId'],
      temp: json['temperature'],
      humidity: json['humidity'],
      level: json['level'],
    );
  }
}

class Sera extends StatelessWidget {
  double _temp, _humidity;
  int _waterLevel;
  Text gh = null;

  Sera(int ghId, double temp, double humidity, int level) {
    if (ghId == 0) {
      gh = new Text(
        "Sera 1",
        style: TextStyle(fontSize: 20),
      );
    } else if (ghId == 1) {
      gh = new Text(
        "Sera 2",
        style: TextStyle(fontSize: 20),
      );
    } else {
      gh = new Text(
        "Sera 3",
        style: TextStyle(fontSize: 20),
      );
    }
    _temp = temp;
    _humidity = humidity;
    _waterLevel = level;
  }

  @override
  Widget build(BuildContext context) {
    return new Container(
      //padding: EdgeInsets.symmetric(horizontal: 0.0),
      child: new Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: <Widget>[
          gh,
          Expanded(
            child: Container(
              padding: EdgeInsets.only(right: 50),
            ),
          ),
          new Icon(Icons.wb_sunny),
          new Text(
            _temp.toString(),
            style: TextStyle(fontSize: 20),
          ),
          Expanded(
            child: Container(
              padding: EdgeInsets.only(right: 50),
            ),
          ),
          new Icon(Icons.wb_sunny),
          new Text(
            _humidity.toString(),
            style: TextStyle(fontSize: 20),
          ),
          Expanded(
            child: Container(
              padding: EdgeInsets.only(right: 50),
            ),
          ),
          new Icon(Icons.storage),
          new Text(
            _waterLevel.toString(),
            style: TextStyle(fontSize: 20),
          ),
        ],
      ),
    );
  }
}
