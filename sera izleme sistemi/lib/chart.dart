import 'package:charts_flutter/flutter.dart' as charts;
import 'package:flutter/material.dart';

class ChartInfos {
  static List<String> ghName = <String>["Sera 1", "Sera 2", "Sera 3"];
  static List<String> chartFreq = <String>["Günlük", "Haftalık", "Aylık"];
  String name = ghName.elementAt(0);
  String freq = chartFreq.elementAt(0);
}

/// Sample time series data type.
class TimeSeriesSales {
  final DateTime time;
  final int sales;

  TimeSeriesSales(this.time, this.sales);
}

class GreenHouseData {
  final int id;
  final DateTime time;
  final double temp;
  final double humidity;
  final int level;

  GreenHouseData(this.id, this.time, this.temp, this.humidity, this.level);
}

class Charts extends StatelessWidget {
  final bool animate;
  List<GreenHouseData> data;
  List<GreenHouseData> _gh0 = new List(), _gh1 = new List(), _gh2 = new List();
  //Charts(this.seriesList, {this.animate});

  Charts(this.data, this.animate) {

    data.forEach((el) {
      if (el.id == 0) {
        _gh0.add(el);
      } else if (el.id == 1) {
        _gh1.add(el);
      } else {
        _gh2.add(el);
      }
    });
  }

  _getDaily(List<GreenHouseData> gh) {
    int size = gh.length;
    size = size - size % 6;
    double temp = 0, hum = 0, level = 0;

    int id = gh.elementAt(0).id;

    int day = 1;
    List<GreenHouseData> tempList = new List();
    for (int i = 0; i < size; ++i) {
      GreenHouseData tempData = gh.elementAt(i);
      if (i % 6 == 5) {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;

        temp /= 6.0;
        hum /= 6.0;
        level /= 6.0;

        tempList.add(new GreenHouseData(
            id, new DateTime(2019, 5, day), temp, hum, level.toInt()));
        ++day;
        temp = 0;
        hum = 0;
        level = 0;
      } else {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;
      }
    }

    return [
      charts.Series<GreenHouseData, DateTime>(
        id: 'Temp',
        colorFn: (_, __) => charts.MaterialPalette.red.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.temp,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Hum',
        colorFn: (_, __) => charts.MaterialPalette.blue.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.humidity,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Level',
        colorFn: (_, __) => charts.MaterialPalette.green.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.level,
        data: tempList,
      ),
    ];
  }

  _getWeekly(List<GreenHouseData> gh) {
    int size = gh.length;
    size = size - size % 42;
    double temp = 0, hum = 0, level = 0;

    int id = gh.elementAt(0).id;

    int day = 1;
    int month = 5;
    List<GreenHouseData> tempList = new List();
    for (int i = 0; i < size; ++i) {
      GreenHouseData tempData = gh.elementAt(i);
      if (i % 42 == 41) {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;

        temp /= 42.0;
        hum /= 42.0;
        level /= 42.0;

        tempList.add(new GreenHouseData(
            id, new DateTime(2019, month, day), temp, hum, level.toInt()));
        day+=7;
        if(day >= 30){
          day=1;
          ++month;
        }
        temp = 0;
        hum = 0;
        level = 0;
      } else {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;
      }
    }

    return [
      charts.Series<GreenHouseData, DateTime>(
        id: 'Temp',
        colorFn: (_, __) => charts.MaterialPalette.red.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.temp,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Hum',
        colorFn: (_, __) => charts.MaterialPalette.blue.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.humidity,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Level',
        colorFn: (_, __) => charts.MaterialPalette.green.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.level,
        data: tempList,
      ),
    ];
  }

  _getMonthly(List<GreenHouseData> gh) {
    int size = gh.length;
    size = size - size % 168;
    double temp = 0, hum = 0, level = 0;

    int id = gh.elementAt(0).id;

    int day = 1;
    int month = 5;
    int year = 2019;
    List<GreenHouseData> tempList = new List();
    for (int i = 0; i < size; ++i) {
      GreenHouseData tempData = gh.elementAt(i);
      if (i % 168 == 167) {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;

        temp /= 168.0;
        hum /= 168.0;
        level /= 168.0;

        tempList.add(new GreenHouseData(
            id, new DateTime(year, month, day), temp, hum, level.toInt()));
        day+=7;
        if(day >= 30){
          day=1;
          ++month;
        }
        if(month >=12 && day >=30){
          year++;
          month =1;
          day=1;
        }
        temp = 0;
        hum = 0;
        level = 0;
      } else {
        temp += tempData.temp;
        hum += tempData.humidity;
        level += tempData.level;
      }
    }

    return [
      charts.Series<GreenHouseData, DateTime>(
        id: 'Temp',
        colorFn: (_, __) => charts.MaterialPalette.red.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.temp,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Hum',
        colorFn: (_, __) => charts.MaterialPalette.blue.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.humidity,
        data: tempList,
      ),
      charts.Series<GreenHouseData, DateTime>(
        id: 'Level',
        colorFn: (_, __) => charts.MaterialPalette.green.shadeDefault,
        domainFn: (GreenHouseData gh, _) => gh.time,
        measureFn: (GreenHouseData gh, _) => gh.level,
        data: tempList,
      ),
    ];
  }

  charts.TimeSeriesChart _getGraph() {
    List<charts.Series<GreenHouseData, DateTime>> list;

    if (_gg.name == "Sera 1" && _gg.freq == "Günlük")
    {
      list = _getDaily(_gh0);
    }
    else if (_gg.name == "Sera 2" && _gg.freq == "Günlük")
    {
      list = _getDaily(_gh1);
    }
    else if (_gg.name == "Sera 3" && _gg.freq == "Günlük")
    {
      list = _getDaily(_gh2);
    }
    else if (_gg.name == "Sera 1" && _gg.freq == "Haftalık")
    {
      list = _getWeekly(_gh0);
    }
    else if (_gg.name == "Sera 2" && _gg.freq == "Haftalık")
    {
      list = _getWeekly(_gh1);
    }
    else if (_gg.name == "Sera 3" && _gg.freq == "Haftalık")
    {
      list = _getWeekly(_gh2);
    }
    else if (_gg.name == "Sera 1" && _gg.freq == "Aylık")
    {
      list = _getMonthly(_gh0);
    }
    else if (_gg.name == "Sera 2" && _gg.freq == "Aylık")
    {
      list = _getMonthly(_gh1);
    }
    else {
      list = _getMonthly(_gh2);
    }



    return new charts.TimeSeriesChart(
      list,
      animate: true,

      // Optionally pass in a [DateTimeFactory] used by the chart. The factory
      // should create the same type of [DateTime] as the data provided. If none
      // specified, the default creates local date time.
      dateTimeFactory: const charts.LocalDateTimeFactory(),
      animationDuration: Duration(milliseconds: 2000),


    );
  }

  ChartInfos _gg = new ChartInfos();

  @override
  Widget build(BuildContext context) {
    return StatefulBuilder(
        builder: (BuildContext context, StateSetter setState) {
      return new MaterialApp(
        theme: new ThemeData(
          primarySwatch: Colors.green,
        ),
        home: new Scaffold(
          appBar: new AppBar(
            title: new Text('Grafikler'),
          ),
          body: new Container(
            padding: EdgeInsets.only(left: 30, right: 30, top: 20),
            child: new ListView(
              children: <Widget>[
                new Row(
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: <Widget>[
                    new DropdownButton<String>(
                        value: _gg.name,
                        items: ChartInfos.ghName.map((String value) {
                          return new DropdownMenuItem(
                              value: value,
                              child: new Row(
                                children: <Widget>[
                                  new Icon(Icons.home),
                                  new Text('Size: $value'),
                                ],
                              ));
                        }).toList(),
                        onChanged: (String value) {
                          setState(() {
                            _gg.name = value;
                            print(_gg.name);
                          });
                        }),
                    new Text("     "),
                    new DropdownButton<String>(
                        value: _gg.freq,
                        items: ChartInfos.chartFreq.map((String value) {
                          return new DropdownMenuItem(
                              value: value,
                              child: new Row(
                                children: <Widget>[
                                  new Icon(Icons.insert_chart),
                                  new Text('Sıklık: $value'),
                                ],
                              ));
                        }).toList(),
                        onChanged: (String value) {
                          setState(() {
                            _gg.freq = value;
                            print(_gg.freq);
                          });
                        })
                  ],
                ),
                Container(
                  child: _getGraph(),
                  constraints: BoxConstraints(
                      maxHeight: 400.0,
                      maxWidth: 300.0,
                      minWidth: 200,
                      minHeight: 200),
                ),
              ],
            ),
          ),
        ),
      );
    });
  }
}
