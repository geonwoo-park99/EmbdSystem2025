# -*- coding: utf-8 -*-
import sys
import struct
import subprocess
import time
import random
import math
BTN1 = 102   
BTN2 = 158   
BTN3 = 217   #Search
BTN4 = 139   #Menu
BTN5 = 115   #Vol.UP
BTN6 = 114   #Vol.Down
from PyQt4.QtGui import QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, QFont, QStackedWidget, QSizePolicy, QMovie, QPainter, QPen, QColor, QPixmap
from PyQt4.QtCore import Qt, QThread, pyqtSignal,QTimer,QSize
from PyQt4.QtGui import QPushButton, QSizePolicy, QSpacerItem
from PyQt4.QtCore import Qt

def read_gyro_subprocess():
    try:
        output = subprocess.check_output(["./gyro_reader"]).decode()
        x, y = map(int, output.strip().split())
        return x, y
    except Exception as e:
        print("자이로 읽기 실패:", e)
        return 0, 0


# 초기 Mode 선택 화면 (화면 0)
class ModeSelector(QWidget):
    def __init__(self, switch_func):
        super(ModeSelector, self).__init__()

        layout = QVBoxLayout()
        layout.setSpacing(10) 
        layout.setAlignment(Qt.AlignCenter) 

        layout.addSpacerItem(QSpacerItem(20, 80, QSizePolicy.Minimum, QSizePolicy.Expanding))

        button_style = """
            QPushButton {
                font-size: 20px;
                padding: 15px;
                min-width: 300px;
                min-height: 60px;
            }
        """

        self.alarm_btn = QPushButton("Alarm Mode")
        self.alarm_btn.setStyleSheet(button_style)
        self.alarm_btn.clicked.connect(lambda: switch_func("alarm"))
        layout.addWidget(self.alarm_btn, alignment=Qt.AlignCenter)

        self.timer_btn = QPushButton("Timer Mode")
        self.timer_btn.setStyleSheet(button_style)
        self.timer_btn.clicked.connect(lambda: switch_func("timer"))
        layout.addWidget(self.timer_btn, alignment=Qt.AlignCenter)

        self.stopwatch_btn = QPushButton("Stopwatch Mode")
        self.stopwatch_btn.setStyleSheet(button_style)
        self.stopwatch_btn.clicked.connect(lambda: switch_func("stopwatch"))
        layout.addWidget(self.stopwatch_btn, alignment=Qt.AlignCenter)

        layout.addSpacerItem(QSpacerItem(20, 80, QSizePolicy.Minimum, QSizePolicy.Expanding))

        self.setLayout(layout)


# 사격 게임 화면
class TargetWidget(QWidget):
    def __init__(self):
        super(TargetWidget, self).__init__()
        self.setWindowTitle("Target Crosshair")
        self.resize(1024, 600)

        # QMovie로 GIF 준비 (QLabel 사용 안 함)
        self.movie = QMovie("/home/ecube/grass.gif")
        self.movie.setScaledSize(QSize(1024, 600))
        self.movie.start()


        # 2. PNG 이미지 (예: 표적지)
        self.png_img = QPixmap("/home/ecube/pngwing.com.png")  # PNG 경로

        # 3. 크로스헤어 위치 등
        self.scale = 0.25
        self.cx = self.width() // 2 + 100
        self.cy = self.height() // 2 + 60
        self.offset_x = 0
        self.offset_y = 0

        self.gyro_thread = GyroThread()
        self.gyro_thread.new_position.connect(self.update_crosshair)
        self.gyro_thread.start()

        self.shots = []
        self.total_score = 0
        self.remaining_shots = 5

        self.result_label = QLabel("", self)
        self.result_label.setAlignment(Qt.AlignCenter)
        self.result_label.setFont(QFont("Arial", 36, QFont.Bold))
        self.result_label.setStyleSheet("color: white; background-color: rgba(0, 0, 0, 160);")
        self.result_label.setGeometry(262, 250, 500, 100)  # 위치 조절 가능
        self.result_label.hide()

    def update_crosshair(self, gx, gy):
        sensitivity = 0.09
        max_offset = 350
        self.offset_x = max(-max_offset, min(max_offset, int(-gy * sensitivity))) 
        self.offset_y = max(-max_offset, min(max_offset, int(-gx * sensitivity))) 
        self.update()

    def update_leds(self, count):
        try:
            print("[LED] 실행: ./ledcontrol {}".format(count))
            result = subprocess.call(["./ledcontrol", str(count)])
            print("[LED] 종료 코드: {}".format(result))
        except Exception as e:
            print("LED 업데이트 실패:", e)

    def set_color_led(self, color):
        try:
            subprocess.call(["./colorled", color])
        except Exception as e:
            print("ColorLED 설정 실패:", e)

    def fire_bullet(self, shot_x, shot_y):
        if len(self.shots) >= 5:
            return  # 5발 다 쐈으면 더 이상 발사 불가

        center_x = self.width() // 2
        center_y = self.height() // 2
        dx = shot_x - center_x
        dy = shot_y - center_y
        distance = math.sqrt(dx**2 + dy**2)

        if distance <= 60:
            score = 10  # 노란색
        elif distance <= 110:
            score = 8   # 빨간색
        elif distance <= 160:
            score = 6   # 파란색
        elif distance <= 210:
            score = 4   # 검은색
        elif distance <= 260:
            score = 2   # 흰색
        else:
            score = 0   # 바깥

        self.total_score += score
        self.shots.append((shot_x, shot_y))

        remaining = 5 - len(self.shots)
        self.update_leds(remaining)
        self.update()

        # 5발을 모두 쐈으면 검사
        if len(self.shots) == 5:
            def show_color_led():
                if self.total_score < 30:
                    self.result_label.setText("Retry")
                    self.result_label.show()
                    self.set_color_led("red") 
                    QTimer.singleShot(1500, self.reset_game)
                    QTimer.singleShot(1500, self.result_label.hide)
                else:
                    self.result_label.setText("WakeUp")
                    self.result_label.show()
                    self.set_color_led("green")
                    QTimer.singleShot(1500, self.return_to_main)
                    QTimer.singleShot(1500, self.result_label.hide)

            # 점수 출력 후 0.3초 뒤에 color LED 점등
            QTimer.singleShot(300, show_color_led)

    def return_to_main(self):

        try:
            subprocess.call(["./buzzer_loop", "off"])  # 강제 off
        except Exception as e:
            print("[ERROR] buzzer off failed:", e)

        if hasattr(self, 'buzzer_proc') and self.buzzer_proc:
            self.buzzer_proc.terminate()
            self.buzzer_proc.wait()
            self.buzzer_proc = None

        parent = self.parent()
        while parent and not isinstance(parent, QStackedWidget):
            parent = parent.parent()
        if parent and isinstance(parent, QStackedWidget):
            parent.setCurrentIndex(0)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        gif_frame = self.movie.currentPixmap()
        painter.drawPixmap(0, 0, gif_frame.scaled(self.width(), self.height()))

        x = (self.width() - self.png_img.width()) // 2
        y = (self.height() - self.png_img.height()) // 2
        painter.drawPixmap(x, y, self.png_img)

        cx = self.cx + self.offset_x
        cy = self.cy + self.offset_y
        scale = self.scale

        big_radius = int(150 * scale)
        pen = QPen(QColor(0, 0, 0), max(1, int(8 * scale)))
        painter.setPen(pen)
        painter.drawEllipse(cx - big_radius, cy - big_radius, big_radius * 2, big_radius * 2)

        line_outer = int(180 * scale)
        line_inner = int(70 * scale)
        painter.drawLine(cx - line_outer, cy, cx - line_inner, cy)
        painter.drawLine(cx + line_inner, cy, cx + line_outer, cy)
        painter.drawLine(cx, cy - line_outer, cx, cy - line_inner)
        painter.drawLine(cx, cy + line_inner, cx, cy + line_outer)

        small_radius = int(20 * scale)
        pen = QPen(QColor(255, 0, 0), max(1, int(4 * scale)))
        painter.setPen(pen)
        painter.drawEllipse(cx - small_radius, cy - small_radius, small_radius * 2, small_radius * 2)

        dot_radius = int(7 * scale)
        painter.setBrush(QColor(0, 0, 0))
        painter.setPen(QPen(QColor(0, 0, 0), max(1, int(2 * scale))))
        painter.drawEllipse(cx - dot_radius, cy - dot_radius, dot_radius * 2, dot_radius * 2)

        for sx, sy in self.shots:
            painter.setPen(QPen(QColor(0, 0, 0), 2))  # 검은색, 두께 2
            size = 6  # X자의 크기
            painter.drawLine(sx - size, sy - size, sx + size, sy + size)
            painter.drawLine(sx - size, sy + size, sx + size, sy - size)
        
        painter.setPen(QPen(QColor(0, 0, 0)))
        painter.setFont(QFont("Arial", 24))
        painter.drawText(self.width() - 200, 50, "Score: {}".format(self.total_score))


    def reset_game(self):
        self.shots = []
        self.total_score = 0
        self.remaining_shots = 5
        self.update_leds(self.remaining_shots)
        self.update()

    def closeEvent(self, event):
        self.gyro_thread.stop()
        self.gyro_thread.wait()
        event.accept()


class GyroThread(QThread):
    new_position = pyqtSignal(int, int)

    def __init__(self, parent=None):
        super(GyroThread, self).__init__(parent)
        self.running = True

    def run(self):
        while self.running:
            x, y = read_gyro_subprocess()
            self.new_position.emit(x, y)
            time.sleep(0.05)

    def stop(self):
        self.running = False


# 1. 버튼 이벤트 읽기용 QThread 정의
class SubprocessButtonThread(QThread):
    button_pressed = pyqtSignal(int, int)  # code, value

    def __init__(self, exec_path="./button_reader"):
        super(SubprocessButtonThread, self).__init__()
        self.exec_path = exec_path
        self.running = True

    def run(self):
        try:
            proc = subprocess.Popen(
                [self.exec_path], stdout=subprocess.PIPE, universal_newlines=True
            )
            while self.running:
                line = proc.stdout.readline()
                if not line:
                    break
                parts = line.strip().split()
                if len(parts) == 2:
                    code = int(parts[0])
                    value = int(parts[1])
                    self.button_pressed.emit(code, value)
        except Exception as e:
            print("[ERROR] SubprocessButtonThread:", e)

        
    def stop(self):
        self.running = False

#Alarm 설정 시간에 도달했는지
class AlarmCheckThread(QThread):
    alarm_time_reached = pyqtSignal(str)  # 알람 신호

    def __init__(self, alarm_time_str='00:00:00', parent=None):
        super(AlarmCheckThread, self).__init__(parent)
        self.alarm_time_str = alarm_time_str
        self.running = True
        self.enabled = False  # 처음엔 알람 비활성화

    def run(self):
        while self.running:
            if self.enabled:
                with open('/sys/class/rtc/rtc0/time') as f:
                    rtc_time = f.read().strip()
                rtc_hms = rtc_time[-8:]  # "hh:mm:ss"
                print("[DEBUG] Current time:", rtc_hms, "/ Set an alarm at:", self.alarm_time_str)
                if rtc_hms == self.alarm_time_str:
                    print("[DEBUG] 알람 시간 일치! 알람 발생")
                    self.alarm_time_reached.emit(rtc_hms)
                    self.enabled = False # 알람 한 번 울리고 다시 비활성화 
            time.sleep(1)

    def set_alarm_time(self, alarm_time_str):
        self.alarm_time_str = alarm_time_str
        self.enabled = True  # 알람 활성화

    def stop(self):
        self.running = False

#Alarm 설정 mode (화면 1)
class AlarmScreen(QWidget):
    def __init__(self):
        super(AlarmScreen, self).__init__()

        self.time_digits = [0, 0, 0, 0, 0, 0]
        self.active_index = 0

        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignCenter)
        layout.setSpacing(20)

        self.time_label = QLabel()
        self.time_label.setTextFormat(Qt.RichText)
        self.time_label.setAlignment(Qt.AlignCenter)
        self.time_label.setStyleSheet("font-size: 48px;")
        self.time_label.setText(self.format_time())
        layout.addWidget(self.time_label)

        self.done_button = QPushButton("Press BTN5 to complete the setup.\nAnd Back to Menu")
        self.done_button.setStyleSheet("""
            QPushButton {
                font-size: 18px;
                padding: 20px;
                min-width: 400px;
                min-height: 80px;
            }
        """)
        layout.addWidget(self.done_button, alignment=Qt.AlignCenter)

        self.setLayout(layout)

    def format_time(self):
        h = self.time_digits[0]*10 + self.time_digits[1]
        m = self.time_digits[2]*10 + self.time_digits[3]
        s = self.time_digits[4]*10 + self.time_digits[5]
        time_str = "{:02d}:{:02d}:{:02d}".format(h, m, s)

        highlighted = ""
        for i, digit in enumerate(self.time_digits):
            char = str(digit)
            if i == self.active_index:
                char = "<u>{}</u>".format(char)
            if i in [1, 3]:
                char += ":"
            highlighted += char
        return "<html><body><p style='font-size:48px'>{}</p></body></html>".format(highlighted)

    def update_display(self):
        self.time_label.setText(self.format_time())

    def handle_button(self, btn_num):
        max_values = [2, 9, 5, 9, 5, 9]  # 기본 최대값 설정 (시, 분, 초)

        # 시의 십의 자리 제한: 0~2 (24시간제)
        if self.active_index == 0:
            max_values[0] = 2
        # 시의 일의 자리 제한: 십의자리에 따라 다름
        elif self.active_index == 1:
            if self.time_digits[0] == 2:
                max_values[1] = 3  # 20~23시만 허용
            else:
                max_values[1] = 9
        # 분, 초의 십의 자리는 항상 0~5로 제한

        if btn_num == 1:
            self.active_index = (self.active_index - 1) % 6
        elif btn_num == 2:
            self.active_index = (self.active_index + 1) % 6
        elif btn_num == 3:
            self.time_digits[self.active_index] = (self.time_digits[self.active_index] + 1) % (max_values[self.active_index] + 1)
        elif btn_num == 4:
            self.time_digits[self.active_index] = (self.time_digits[self.active_index] - 1) % (max_values[self.active_index] + 1)
        self.update_display()

    def get_alarm_time_str(self):
        h = self.time_digits[0]*10 + self.time_digits[1]
        m = self.time_digits[2]*10 + self.time_digits[3]
        s = self.time_digits[4]*10 + self.time_digits[5]
        return "{:02d}:{:02d}:{:02d}".format(h, m, s)

#Timer 설정 mode (화면 2)
class TimerSettingWidget(QWidget):
    def __init__(self):
        super(TimerSettingWidget, self).__init__()
        self.time_digits = [0, 0, 0, 0]  # mm:ss 구조
        self.active_index = 0  # 0~3 인덱스

        self.countdown_timer = QTimer(self)
        self.countdown_timer.timeout.connect(self.update_countdown)
        self.remaining_seconds = 0
        self.timer_running = False  # or False
        self.in_countdown_mode = False
        self.external_switch_func = None

        layout = QVBoxLayout()
        layout.setAlignment(Qt.AlignCenter)
        layout.setSpacing(20)

        # 타이머 시간 표시
        self.time_label = QLabel()
        self.time_label.setTextFormat(Qt.RichText)
        self.time_label.setAlignment(Qt.AlignCenter)
        self.time_label.setStyleSheet("font-size: 48px;")
        self.time_label.setText(self.format_time())
        layout.addWidget(self.time_label)

        # 버튼 6개 가로 배치
        button_layout = QHBoxLayout()
        button_labels = ["<-", "->", "+1", "-1", "Start Timer!", "Back to Menu"]
        self.buttons = []

        for label in button_labels:
            btn = QPushButton(label)
            btn.setStyleSheet("""
                QPushButton {
                    font-size: 16px;
                    padding: 8px;
                    min-width: 100px;
                    min-height: 40px;
                }
            """)
            btn.setEnabled(False)  # 버튼은 하드웨어 키로 동작하므로 비활성화
            self.buttons.append(btn)
            button_layout.addWidget(btn)

        layout.addLayout(button_layout)
        self.setLayout(layout)

    def set_screen_switch_callback(self, func):
        self.external_switch_func = func

    def format_time(self):
        m = self.remaining_seconds // 60 if self.in_countdown_mode else self.time_digits[0]*10 + self.time_digits[1]
        s = self.remaining_seconds % 60 if self.in_countdown_mode else self.time_digits[2]*10 + self.time_digits[3]
        time_str = "{:02}:{:02}".format(m, s)
        
        highlighted = ""
        if not self.in_countdown_mode:
            for i, digit in enumerate(self.time_digits):
                char = str(digit)
                if i == self.active_index:
                    char = "<u>{}</u>".format(char)
                if i == 1:
                    char += ":"
                highlighted += char
            return "<html><body><p style='font-size:48px'>{}</p></body></html>".format(highlighted)
        else:
            return "<html><body><p style='font-size:48px'>{}</p></body></html>".format(time_str)


    def update_display(self):
        self.time_label.setText(self.format_time())

    def handle_button(self, btn_num):
        max_values = [5, 9, 5, 9]  # mm:ss에서 분/초 십의자리: 0~5, 일의자리: 0~9
        if not self.in_countdown_mode:
            if btn_num == 1:
                self.active_index = (self.active_index - 1) % 4
            elif btn_num == 2:
                self.active_index = (self.active_index + 1) % 4
            elif btn_num == 3:
                self.time_digits[self.active_index] = (self.time_digits[self.active_index] + 1) % (max_values[self.active_index] + 1)
            elif btn_num == 4:
                self.time_digits[self.active_index] = (self.time_digits[self.active_index] - 1) % (max_values[self.active_index] + 1)
            elif btn_num == 5:
                self.start_countdown()
            elif btn_num == 6:
                if self.external_switch_func:
                    self.external_switch_func(0)  # 메뉴로 돌아감
        else:
            if btn_num == 5:  # Stop/Continue
                self.timer_running = not self.timer_running

            elif btn_num == 6:
                self.stop_countdown()

        self.update_display()
    
    def start_countdown(self):
        self.remaining_seconds = self.get_timer_seconds()
        if self.remaining_seconds <= 0:
            return  # 00:00이면 무시
        self.in_countdown_mode = True
        self.timer_running = True
        self.countdown_timer.start(1000)

        self.buttons[4].setText("Stop/Continue")
        self.buttons[5].setText("Reset")

    def stop_countdown(self):
        self.countdown_timer.stop()
        self.in_countdown_mode = False
        self.timer_running = False
        self.remaining_seconds = 0
        self.buttons[4].setText("Start Timer!")
        self.buttons[5].setText("Back to Menu")

    def update_countdown(self):
        if self.timer_running:
            if self.remaining_seconds > 0:
                self.remaining_seconds -= 1
                self.update_display()
            if self.remaining_seconds == 0 and not getattr(self, 'entered_game', False):
                self.entered_game = True
                self.countdown_timer.stop()
                print("Countdown finished. Switching to shooting game.")
                if self.external_switch_func:
                    self.external_switch_func(3)  # 사격 게임으로 전환
                else:
                    print("[ERROR] external_switch_func is not set!")


    def get_timer_seconds(self):
        m = self.time_digits[0]*10 + self.time_digits[1]
        s = self.time_digits[2]*10 + self.time_digits[3]
        return m * 60 + s

class StopwatchWidget(QWidget):
    def __init__(self):
        super(StopwatchWidget, self).__init__()
        self.stopwatch_seconds = 0
        self.running = False
        self.in_countup_mode = False
        self.external_switch_func = None

        self.timer = QTimer(self)
        self.timer.timeout.connect(self.update_stopwatch)

        self.layout = QVBoxLayout()
        self.layout.setAlignment(Qt.AlignCenter)
        self.layout.setSpacing(20)

        self.time_label = QLabel()
        self.time_label.setTextFormat(Qt.RichText)
        self.time_label.setAlignment(Qt.AlignCenter)
        self.time_label.setStyleSheet("font-size: 48px;")
        self.time_label.setText("00:00")
        self.layout.addWidget(self.time_label)

        self.button_layout = QHBoxLayout()
        self.buttons = []

        self.setLayout(self.layout)
        self.show_setting_buttons()

    def set_screen_switch_callback(self, func):
        self.external_switch_func = func

    def format_time(self):
        m = self.stopwatch_seconds // 60
        s = self.stopwatch_seconds % 60
        return "{:02}:{:02}".format(m, s)

    def update_display(self):
        self.time_label.setText("<html><body><p style='font-size:48px'>{}</p></body></html>".format(self.format_time()))

    def update_stopwatch(self):
        if self.running:
            self.stopwatch_seconds += 1
            self.update_display()

    def show_setting_buttons(self):
        self.in_countup_mode = False
        self.clear_buttons()

        btn1 = QPushButton("Start Stopwatch!")
        btn2 = QPushButton("Back to Menu")
        for btn in [btn1, btn2]:
            btn.setStyleSheet("font-size: 16px; min-width: 140px; min-height: 40px;")
            btn.setEnabled(False)
        self.buttons = [btn1, btn2]
        for b in self.buttons:
            self.button_layout.addWidget(b)
        self.layout.addLayout(self.button_layout)

    def show_countup_buttons(self):
        self.in_countup_mode = True
        self.clear_buttons()

        btn1 = QPushButton("Stop/Continue")
        btn2 = QPushButton("Reset")
        btn3 = QPushButton("Record")
        for btn in [btn1, btn2, btn3]:
            btn.setStyleSheet("font-size: 16px; min-width: 100px; min-height: 40px;")
            btn.setEnabled(False)
        self.buttons = [btn1, btn2, btn3]
        for b in self.buttons:
            self.button_layout.addWidget(b)
        self.layout.addLayout(self.button_layout)

    def clear_buttons(self):
        for btn in self.buttons:
            self.button_layout.removeWidget(btn)
            btn.deleteLater()
        self.buttons = []

    def handle_button(self, btn_num):
        if not self.in_countup_mode:
            if btn_num == 1:
                self.start_stopwatch()
            elif btn_num == 2:
                self.reset_stopwatch()
                import subprocess
                subprocess.call(["./lcd_print", ""]) 
                if self.external_switch_func:
                    self.external_switch_func(0)
        else:
            if btn_num == 1:
                self.toggle_running()
            elif btn_num == 2:
                self.stopwatch_seconds = 0
                self.timer.stop()
                self.show_setting_buttons()
                self.update_display()
            elif btn_num == 3:
                print("[RECORD] Time:", self.format_time())
                import subprocess
                subprocess.call(["./lcd_print", "[RECORD] " + self.format_time()])

    def start_stopwatch(self):
        self.stopwatch_seconds = 0
        self.running = True
        self.in_countup_mode = True
        self.update_display()
        self.timer.start(1000)
        self.show_countup_buttons()

    def toggle_running(self):
        self.running = not self.running

    def reset_stopwatch(self):
        self.timer.stop()
        self.stopwatch_seconds = 0
        self.running = False
        self.in_countup_mode = False
        self.update_display()

# 2. 메인 위젯
class SimpleTextWidget(QWidget):
    def __init__(self):
        super(SimpleTextWidget, self).__init__()

        self.buzzer_proc = None
       
        self.alarm_widget = AlarmScreen()
        self.timer_widget = TimerSettingWidget()
        self.timer_widget.set_screen_switch_callback(self.switch_mode)
        self.main_label = QLabel(self)
        self.main_label.setAlignment(Qt.AlignCenter)
        self.main_label.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.target_widget = TargetWidget() 
        self.fnd_proc = subprocess.Popen(["./fnd_clock"]) 



        # 알람 변수
        self.alarm_is_set = True
        self.alarm_hour = 7
        self.alarm_minute = 30
        self.alarm_second = 0


        # 알람 시간 문자열 만들기 (두 자리로 맞춤)
        self.alarm_time_str = "{:02d}:{:02d}:{:02d}".format(self.alarm_hour, self.alarm_minute, self.alarm_second)
        self.alarm_text = "Alarm: ON" if self.alarm_is_set else "Alarm: OFF"

        self.mode_selector = ModeSelector(self.switch_mode)
        self.stopwatch_widget = StopwatchWidget()
        self.stopwatch_widget.set_screen_switch_callback(self.switch_mode)

        self.stacked = QStackedWidget()
        self.stacked.addWidget(self.mode_selector)      # index 0
        self.stacked.addWidget(self.alarm_widget)       # index 1
        self.stacked.addWidget(self.timer_widget)       # index 2
        self.stacked.addWidget(self.target_widget)      # index 3
        self.stacked.addWidget(self.stopwatch_widget)   # index 4

        
        self.setWindowTitle("alarm")
        self.resize(1024, 600)


        layout = QVBoxLayout()
        layout.addWidget(self.stacked)
        self.setLayout(layout)

        # 3. 버튼 이벤트 스레드 시작
        self.button_thread = SubprocessButtonThread('./button_reader')
        self.button_thread.button_pressed.connect(self.on_button_pressed)
        self.button_thread.start()
       
        #4.rtc 알람 스레드
        self.alarm_thread = AlarmCheckThread('01')
        self.alarm_thread.alarm_time_reached.connect(self.on_alarm_time)
        self.alarm_thread.start()

    def switch_mode(self, mode): 
        if mode == "alarm" or mode == 1:
            self.stacked.setCurrentIndex(1)
        elif mode == "timer" or mode == 2:
            self.stacked.setCurrentIndex(2)
        elif mode == "stopwatch" or mode == 3:
            self.buzzer_proc = subprocess.Popen(["./buzzer_loop"])
            self.target_widget.buzzer_proc = self.buzzer_proc
            self.target_widget.reset_game()  
            self.stacked.setCurrentIndex(3)
        elif mode == 4:
            self.stacked.setCurrentIndex(4) 
        elif mode == 0:
            self.stacked.setCurrentIndex(0)  # 메뉴로 돌아가는 경우

    def on_alarm_time(self, rtc_time):
        # 알람 시간이 되면 화면을 전환!
        self.buzzer_proc = subprocess.Popen(["./buzzer_loop"])
        self.target_widget.buzzer_proc = self.buzzer_proc
        self.target_widget.reset_game()
        self.stacked.setCurrentIndex(3)  # 예: 1번(알람 설정 화면)으로 전환
        self.alarm_text = "Alarm: OFF"          # 알람 상태를 OFF로 변경
        print("알람 화면으로 전환!")

    def on_button_pressed(self, code, value):
        # 버튼이 눌릴 때마다 호출 (code: 키코드, value: 1=눌림, 0=뗌)
        #print(f"Button code={code}, value={value}")
        # 예시: 특정 버튼이 눌리면 알람 시간 변경
        if value != 1:
            return
        
        current_index = self.stacked.currentIndex()

        # 화면 0에서만 동작
        if current_index == 0:
            if code == BTN1:
                self.stacked.setCurrentIndex(1)
                return
            elif code == BTN2:
                self.stacked.setCurrentIndex(2)
                return
            elif code == BTN3:
                self.stacked.setCurrentIndex(4)  # ✅ 채널 4로 이동 (Stopwatch)
                return

        # 화면 1에서만 동작
        if current_index == 1:
            if code == BTN1:  # 예: 버튼 1 (←)
                self.alarm_widget.handle_button(1)
            elif code == BTN2:  # 예: 버튼 2 (→)
                self.alarm_widget.handle_button(2)
            elif code == BTN3:
                self.alarm_widget.handle_button(3)  # ↑
            elif code == BTN4:
                self.alarm_widget.handle_button(4)  # ↓
            elif code == BTN5:
                new_alarm_time = self.alarm_widget.get_alarm_time_str()
                self.alarm_thread.set_alarm_time(new_alarm_time)
                print("Alarm is set:", new_alarm_time)
                self.stacked.setCurrentIndex(0)  # 메인 메뉴로 이동
            elif code == BTN6:
                self.alarm_widget.time_digits = [0, 0, 0, 0, 0, 0]
                self.alarm_widget.update_display()
                self.alarm_thread.set_alarm_time("00:00:00")
                self.alarm_thread.enabled = False  # 알람 비활성화
                self.stacked.setCurrentIndex(0)  # 메인 메뉴로 이동
                print("Alarm time reset and alarm disabled.")
        
        # 화면 2에서만 동작
        if current_index == 2:
            if code == BTN1:
                self.timer_widget.handle_button(1)
            elif code == BTN2:
                self.timer_widget.handle_button(2)
            elif code == BTN3:
                self.timer_widget.handle_button(3)
            elif code == BTN4:
                self.timer_widget.handle_button(4)
            elif code == BTN5:
                seconds = self.timer_widget.get_timer_seconds()
                print("Start Timer! {}초".format(seconds))
                self.timer_widget.handle_button(5)  # 타이머 시작!
            elif code == BTN6:
                if self.timer_widget.in_countdown_mode:
                    self.timer_widget.handle_button(6)  # stop_countdown()만 수행
                else:
                    self.timer_widget.time_digits = [0, 0, 0, 0]
                    self.timer_widget.remaining_seconds = 0       # 🔧 남은 시간도 초기화
                    self.timer_widget.in_countdown_mode = False
                    self.timer_widget.timer_running = False
                    self.timer_widget.update_display()
                    self.timer_widget.buttons[4].setText("Start Timer!")
                    self.timer_widget.buttons[5].setText("Back to Menu")
                    self.stacked.setCurrentIndex(0)  # 설정 모드였으면 메인으로
                    print("Timer setting reset and timer disabled.")

        # 화면 3에서만 동작
        if current_index == 3:
            if code == BTN2:  # BTN2 = Fire
                shot_x = self.target_widget.cx + self.target_widget.offset_x
                shot_y = self.target_widget.cy + self.target_widget.offset_y
                self.target_widget.fire_bullet(shot_x, shot_y)

        # 화면 4에서만 동작
        if current_index == 4:
            if code == BTN1:
                self.stopwatch_widget.handle_button(1)
            elif code == BTN2:
                self.stopwatch_widget.handle_button(2)
            elif code == BTN3:
                self.stopwatch_widget.handle_button(3)

    def set_screen_switch_callback(self, func):
        self.external_switch_func = func

    def closeEvent(self, event):
        # 창 닫힐 때 스레드 종료
        self.button_thread.stop()
        self.button_thread.wait()
        if hasattr(self, 'fnd_proc'):
            self.fnd_proc.terminate()
            self.fnd_proc.wait()
        event.accept()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = SimpleTextWidget()
    window.show()
    sys.exit(app.exec_())
