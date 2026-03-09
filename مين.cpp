import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_bluetooth_serial/flutter_bluetooth_serial.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:fl_chart/fl_chart.dart';
import 'package:google_fonts/google_fonts.dart';
import 'package:shared_preferences/shared_preferences.dart';

void main() {
  runApp(const IncubatorApp());
}

// ===== أنواع البيض =====
class EggMode {
  final String id, name, emoji;
  final double temp;
  final int humMin, humMax, days, stopTurnDay, turnHours;
  final List<String> phases;

  const EggMode({
    required this.id, required this.name, required this.emoji,
    required this.temp, required this.humMin, required this.humMax,
    required this.days, required this.stopTurnDay, required this.turnHours,
    required this.phases,
  });
}

const List<EggMode> eggModes = [
  EggMode(id:'chicken', name:'بيض دجاج',     emoji:'🐔', temp:37.5, humMin:55, humMax:65, days:21, stopTurnDay:18, turnHours:8,  phases:['بداية','نمو','إغلاق','فقس']),
  EggMode(id:'quail',   name:'بيض سمان',     emoji:'🐦', temp:37.8, humMin:45, humMax:60, days:17, stopTurnDay:14, turnHours:6,  phases:['بداية','نمو','فقس']),
  EggMode(id:'duck',    name:'بيض بط',       emoji:'🦆', temp:37.5, humMin:65, humMax:75, days:28, stopTurnDay:25, turnHours:8,  phases:['بداية','نمو','إغلاق','فقس']),
  EggMode(id:'turkey',  name:'ديك رومي',     emoji:'🦃', temp:37.5, humMin:55, humMax:65, days:28, stopTurnDay:25, turnHours:8,  phases:['بداية','نمو','إغلاق','فقس']),
  EggMode(id:'goose',   name:'بيض وز',       emoji:'🪿', temp:37.4, humMin:60, humMax:75, days:30, stopTurnDay:27, turnHours:8,  phases:['بداية','نمو','إغلاق','فقس']),
  EggMode(id:'pigeon',  name:'بيض حمام',     emoji:'🕊️', temp:37.8, humMin:50, humMax:60, days:18, stopTurnDay:15, turnHours:6,  phases:['بداية','نمو','فقس']),
];

// ===== ألوان =====
const Color bgColor      = Color(0xFF0A0E17);
const Color surfaceColor = Color(0xFF111827);
const Color surface2     = Color(0xFF1A2332);
const Color accentColor  = Color(0xFFF59E0B);
const Color redColor     = Color(0xFFEF4444);
const Color greenColor   = Color(0xFF10B981);
const Color blueColor    = Color(0xFF3B82F6);
const Color textColor    = Color(0xFFF1F5F9);
const Color text2Color   = Color(0xFF94A3B8);
const Color text3Color   = Color(0xFF475569);

class IncubatorApp extends StatelessWidget {
  const IncubatorApp({super.key});
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'فقاسة ذكية',
      debugShowCheckedModeBanner: false,
      theme: ThemeData(
        brightness: Brightness.dark,
        scaffoldBackgroundColor: bgColor,
        colorScheme: const ColorScheme.dark(primary: accentColor, surface: surfaceColor),
        textTheme: GoogleFonts.cairoTextTheme(ThemeData.dark().textTheme),
        useMaterial3: true,
      ),
      home: const HomePage(),
    );
  }
}

// ===== HOME PAGE =====
class HomePage extends StatefulWidget {
  const HomePage({super.key});
  @override
  State<HomePage> createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  int _currentIndex = 0;
  BluetoothConnection? _connection;
  bool _isConnected = false;
  String _btStatus = 'غير متصل';

  // بيانات الحساسات
  double temp = 0, hum = 0;
  bool heatOn = false, fanOn = false, turnOn = true;
  int currentDay = 1;
  List<FlSpot> tempHistory = [];
  List<Map<String, String>> logs = [];

  // إعدادات
  EggMode selectedMode = eggModes[0];
  DateTime incubationStart = DateTime.now();

  // بفر البيانات الواردة
  String _dataBuffer = '';

  @override
  void initState() {
    super.initState();
    _loadPrefs();
    _requestPermissions();
  }

  Future<void> _loadPrefs() async {
    final prefs = await SharedPreferences.getInstance();
    final modeId = prefs.getString('mode') ?? 'chicken';
    final startMs = prefs.getInt('start') ?? DateTime.now().millisecondsSinceEpoch;
    setState(() {
      selectedMode = eggModes.firstWhere((m) => m.id == modeId, orElse: () => eggModes[0]);
      incubationStart = DateTime.fromMillisecondsSinceEpoch(startMs);
    });
  }

  Future<void> _savePrefs() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('mode', selectedMode.id);
    await prefs.setInt('start', incubationStart.millisecondsSinceEpoch);
  }

  Future<void> _requestPermissions() async {
    await [
      Permission.bluetooth,
      Permission.bluetoothConnect,
      Permission.bluetoothScan,
      Permission.location,
    ].request();
  }

  // ===== اتصال البلوتوث =====
  Future<void> _connectBluetooth() async {
    try {
      setState(() => _btStatus = 'جاري البحث...');
      final devices = await FlutterBluetoothSerial.instance.getBondedDevices();
      final hc06 = devices.firstWhere(
        (d) => d.name == 'HC-06' || d.name == 'HC-05',
        orElse: () => throw Exception('HC-06 مش موجود في الأجهزة المقترنة'),
      );
      setState(() => _btStatus = 'جاري الاتصال...');
      final conn = await BluetoothConnection.toAddress(hc06.address);
      setState(() {
        _connection = conn;
        _isConnected = true;
        _btStatus = 'متصل بـ ${hc06.name}';
      });
      _addLog('✅ متصل بـ ${hc06.name}', 'ok');

      // استقبال البيانات
      conn.input!.listen((data) {
        _dataBuffer += ascii.decode(data);
        while (_dataBuffer.contains('\n')) {
          final idx = _dataBuffer.indexOf('\n');
          final line = _dataBuffer.substring(0, idx).trim();
          _dataBuffer = _dataBuffer.substring(idx + 1);
          _parseData(line);
        }
      }, onDone: () {
        setState(() { _isConnected = false; _btStatus = 'انقطع الاتصال'; });
        _addLog('انقطع الاتصال', 'warn');
      });
    } catch (e) {
      setState(() => _btStatus = 'فشل: ${e.toString()}');
      _addLog('فشل الاتصال: ${e.toString()}', 'warn');
    }
  }

  void _disconnect() {
    _connection?.dispose();
    setState(() { _isConnected = false; _btStatus = 'غير متصل'; });
  }

  void _sendCmd(String cmd) {
    if (_connection == null || !_isConnected) {
      _addLog('غير متصل — $cmd', 'warn'); return;
    }
    _connection!.output.add(ascii.encode('$cmd\n'));
    _addLog('أُرسل: $cmd', 'info');
  }

  // ===== تحليل البيانات =====
  void _parseData(String raw) {
    if (raw.startsWith('T:')) {
      final parts = <String, String>{};
      for (final p in raw.split(',')) {
        final kv = p.split(':');
        if (kv.length == 2) parts[kv[0]] = kv[1];
      }
      setState(() {
        temp = double.tryParse(parts['T'] ?? '') ?? temp;
        hum  = double.tryParse(parts['H'] ?? '') ?? hum;
        heatOn  = parts['HEAT'] == '1';
        fanOn   = parts['FAN']  == '1';
        turnOn  = parts['TURN'] == '1';
        currentDay = int.tryParse(parts['D'] ?? '') ?? currentDay;

        final x = tempHistory.length.toDouble();
        tempHistory.add(FlSpot(x, temp));
        if (tempHistory.length > 20) {
          tempHistory = tempHistory.sublist(1).asMap().entries
              .map((e) => FlSpot(e.key.toDouble(), e.value.y)).toList();
        }
      });
    }
    if (raw.startsWith('ALERT:')) {
      final msgs = {
        'ALERT:TEMP_HIGH': '🚨 الحرارة عالية جداً!',
        'ALERT:TEMP_LOW':  '🥶 الحرارة منخفضة جداً!',
        'ALERT:HUM_HIGH':  '💧 الرطوبة عالية!',
        'ALERT:HUM_LOW':   '🏜️ الرطوبة منخفضة!',
      };
      _addLog(msgs[raw] ?? raw, 'warn');
    }
    if (raw == 'TURNING_EGGS') _addLog('🔄 جاري تقليب البيض', 'ok');
  }

  void _addLog(String msg, String type) {
    final now = TimeOfDay.now();
    setState(() {
      logs.insert(0, {
        'time': '${now.hour.toString().padLeft(2,'0')}:${now.minute.toString().padLeft(2,'0')}',
        'msg': msg, 'type': type,
      });
      if (logs.length > 50) logs.removeLast();
    });
  }

  // ===== حساب التقدم =====
  int get elapsedDays => DateTime.now().difference(incubationStart).inDays;
  int get remainingDays => (selectedMode.days - elapsedDays).clamp(0, selectedMode.days);
  double get progressPct => (elapsedDays / selectedMode.days).clamp(0.0, 1.0);

  @override
  Widget build(BuildContext context) {
    final pages = [
      _buildHomePage(),
      _buildModesPage(),
      _buildControlPage(),
      _buildSettingsPage(),
    ];

    return Scaffold(
      body: pages[_currentIndex],
      bottomNavigationBar: _buildBottomNav(),
    );
  }

  // ===== BOTTOM NAV =====
  Widget _buildBottomNav() {
    final items = [
      {'icon': Icons.monitor_heart_rounded, 'label': 'لايف'},
      {'icon': Icons.egg_rounded,           'label': 'مودات'},
      {'icon': Icons.gamepad_rounded,       'label': 'تحكم'},
      {'icon': Icons.settings_rounded,      'label': 'إعدادات'},
    ];
    return Container(
      decoration: const BoxDecoration(
        color: Color(0xF00A0E17),
        border: Border(top: BorderSide(color: Color(0x15FFFFFF))),
      ),
      child: SafeArea(
        child: Row(
          children: List.generate(items.length, (i) {
            final active = _currentIndex == i;
            return Expanded(
              child: GestureDetector(
                onTap: () => setState(() => _currentIndex = i),
                behavior: HitTestBehavior.opaque,
                child: Padding(
                  padding: const EdgeInsets.symmetric(vertical: 10),
                  child: Column(
                    mainAxisSize: MainAxisSize.min,
                    children: [
                      Icon(items[i]['icon'] as IconData,
                          color: active ? accentColor : text3Color, size: 24),
                      const SizedBox(height: 4),
                      Text(items[i]['label'] as String,
                          style: TextStyle(
                            fontSize: 11, fontWeight: FontWeight.w600,
                            color: active ? accentColor : text3Color,
                          )),
                    ],
                  ),
                ),
              ),
            );
          }),
        ),
      ),
    );
  }

  // ========================================
  // ===== صفحة لايف =====
  // ========================================
  Widget _buildHomePage() {
    return SafeArea(
      child: CustomScrollView(
        slivers: [
          SliverAppBar(
            backgroundColor: const Color(0xF00A0E17),
            floating: true,
            title: Row(
              children: [
                Container(
                  width: 36, height: 36,
                  decoration: BoxDecoration(
                    gradient: const LinearGradient(colors: [accentColor, Color(0xFFD97706)]),
                    borderRadius: BorderRadius.circular(18),
                    boxShadow: [BoxShadow(color: accentColor.withOpacity(0.4), blurRadius: 12)],
                  ),
                  child: const Center(child: Text('🥚', style: TextStyle(fontSize: 18))),
                ),
                const SizedBox(width: 10),
                Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    Text('فقاسة ذكية', style: GoogleFonts.cairo(fontSize: 16, fontWeight: FontWeight.w700)),
                    Text('${selectedMode.name} · ${selectedMode.days} يوم',
                        style: GoogleFonts.cairo(fontSize: 11, color: text2Color)),
                  ],
                ),
              ],
            ),
            actions: [
              GestureDetector(
                onTap: _isConnected ? _disconnect : _connectBluetooth,
                child: Container(
                  margin: const EdgeInsets.only(left: 16, right: 16),
                  padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 7),
                  decoration: BoxDecoration(
                    color: _isConnected ? greenColor.withOpacity(0.1) : surfaceColor,
                    border: Border.all(
                      color: _isConnected ? greenColor.withOpacity(0.4) : const Color(0x15FFFFFF),
                    ),
                    borderRadius: BorderRadius.circular(10),
                  ),
                  child: Row(
                    children: [
                      Container(
                        width: 8, height: 8,
                        decoration: BoxDecoration(
                          color: _isConnected ? greenColor : text3Color,
                          shape: BoxShape.circle,
                          boxShadow: _isConnected
                              ? [BoxShadow(color: greenColor.withOpacity(0.6), blurRadius: 6)]
                              : null,
                        ),
                      ),
                      const SizedBox(width: 6),
                      Text(_isConnected ? 'متصل' : 'اتصال',
                          style: GoogleFonts.cairo(
                              fontSize: 12,
                              color: _isConnected ? greenColor : text2Color,
                              fontWeight: FontWeight.w600)),
                    ],
                  ),
                ),
              ),
            ],
          ),
          SliverPadding(
            padding: const EdgeInsets.all(16),
            sliver: SliverList(delegate: SliverChildListDelegate([
              // BT Status
              _btStatusBar(),
              const SizedBox(height: 12),

              // Temp & Hum cards
              Row(children: [
                Expanded(child: _metricCard(
                  label: '🌡️ الحرارة',
                  value: _isConnected ? temp.toStringAsFixed(1) : '--',
                  unit: '°C',
                  color: accentColor,
                  pct: ((temp - 35) / 5).clamp(0, 1).toDouble(),
                )),
                const SizedBox(width: 12),
                Expanded(child: _metricCard(
                  label: '💧 الرطوبة',
                  value: _isConnected ? hum.toStringAsFixed(1) : '--',
                  unit: '%',
                  color: blueColor,
                  pct: (hum / 100).clamp(0, 1).toDouble(),
                )),
              ]),
              const SizedBox(height: 12),

              // Progress
              _progressCard(),
              const SizedBox(height: 12),

              // Status
              Row(children: [
                Expanded(child: _statusCard('🔆', 'اللمبة', heatOn ? 'تشتغل 🔥' : 'متوقفة', heatOn, accentColor)),
                const SizedBox(width: 12),
                Expanded(child: _statusCard('🌀', 'المروحة', fanOn ? 'تشتغل 💨' : 'متوقفة', fanOn, blueColor)),
              ]),
              const SizedBox(height: 12),

              // Alerts
              if (_isConnected) ..._buildAlerts(),

              // Chart
              _chartCard(),
              const SizedBox(height: 80),
            ])),
          ),
        ],
      ),
    );
  }

  Widget _btStatusBar() {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 11),
      decoration: BoxDecoration(
        color: (_isConnected ? greenColor : accentColor).withOpacity(0.08),
        border: Border.all(color: (_isConnected ? greenColor : accentColor).withOpacity(0.2)),
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          Text(_isConnected ? '✅' : '📶', style: const TextStyle(fontSize: 16)),
          const SizedBox(width: 8),
          Expanded(child: Text(_btStatus,
              style: GoogleFonts.cairo(
                  fontSize: 13,
                  color: _isConnected ? greenColor : accentColor))),
        ],
      ),
    );
  }

  Widget _metricCard({required String label, required String value, required String unit,
      required Color color, required double pct}) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: surfaceColor,
        borderRadius: BorderRadius.circular(16),
        border: Border.all(color: const Color(0x12FFFFFF)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(label, style: GoogleFonts.cairo(fontSize: 11, color: text2Color)),
          const SizedBox(height: 8),
          RichText(text: TextSpan(
            text: value,
            style: GoogleFonts.tajawal(fontSize: 28, fontWeight: FontWeight.w900, color: color),
            children: [TextSpan(text: unit,
                style: GoogleFonts.cairo(fontSize: 13, color: text2Color, fontWeight: FontWeight.normal))],
          )),
          const SizedBox(height: 10),
          ClipRRect(
            borderRadius: BorderRadius.circular(2),
            child: LinearProgressIndicator(
              value: pct, minHeight: 3,
              backgroundColor: surface2,
              valueColor: AlwaysStoppedAnimation(color),
            ),
          ),
        ],
      ),
    );
  }

  Widget _progressCard() {
    final phaseCount = selectedMode.phases.length;
    final phaseSize = selectedMode.days / phaseCount;
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: surfaceColor, borderRadius: BorderRadius.circular(16),
        border: Border.all(color: const Color(0x12FFFFFF)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text('🕐 تقدم الحضانة', style: GoogleFonts.cairo(fontSize: 15, fontWeight: FontWeight.w700)),
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                decoration: BoxDecoration(
                  color: accentColor.withOpacity(0.15),
                  border: Border.all(color: accentColor.withOpacity(0.3)),
                  borderRadius: BorderRadius.circular(8),
                ),
                child: Text('يوم ${elapsedDays + 1}',
                    style: GoogleFonts.cairo(fontSize: 13, fontWeight: FontWeight.w700, color: accentColor)),
              ),
            ],
          ),
          const SizedBox(height: 10),
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              Text('مر $elapsedDays أيام من ${selectedMode.days}',
                  style: GoogleFonts.cairo(fontSize: 12, color: text2Color)),
              Text(remainingDays == 0 ? '🎉 حان وقت الفقس!' : 'متبقي $remainingDays يوم',
                  style: GoogleFonts.cairo(fontSize: 12, color: accentColor, fontWeight: FontWeight.w600)),
            ],
          ),
          const SizedBox(height: 8),
          ClipRRect(
            borderRadius: BorderRadius.circular(4),
            child: LinearProgressIndicator(
              value: progressPct, minHeight: 8,
              backgroundColor: surface2,
              valueColor: const AlwaysStoppedAnimation(accentColor),
            ),
          ),
          const SizedBox(height: 12),
          Wrap(
            spacing: 6, runSpacing: 6,
            children: List.generate(phaseCount, (i) {
              final pStart = (i * phaseSize).floor();
              final pEnd = (i == phaseCount - 1) ? selectedMode.days : ((i + 1) * phaseSize).floor();
              Color c; String bg;
              if (elapsedDays >= pEnd) { c = greenColor; bg = 'done'; }
              else if (elapsedDays >= pStart) { c = accentColor; bg = 'cur'; }
              else { c = text3Color; bg = 'pend'; }
              return Container(
                padding: const EdgeInsets.symmetric(horizontal: 9, vertical: 3),
                decoration: BoxDecoration(
                  color: c.withOpacity(0.1), borderRadius: BorderRadius.circular(6),
                  border: Border.all(color: c.withOpacity(0.25)),
                ),
                child: Text(selectedMode.phases[i],
                    style: GoogleFonts.cairo(fontSize: 11, color: c, fontWeight: Fo
