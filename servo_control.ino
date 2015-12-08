#include <Servo.h>

class MotorMotion
{
  private:
    int _target;
    int _pos;
    int _step;
    int _delay;
    int _pass;
    bool _bActive;
    Servo _servo;
  public:
    MotorMotion()
    {
      _bActive = false;
    }
    
    void Attach(int nPin)
    {
      _servo.attach(nPin);
      _pos = 1500;
      _servo.writeMicroseconds(_pos);
    }
    
    void MotionTo(int target)
    {
      _bActive = true;
      _target = target;
      if(_target > _pos)
        _step = 1;
      else
        _step = -1;
      _pass = _delay;
    }

    void setDelay(int nDelay)
    {
      _delay = nDelay;
    }

    void gotoDirect(int us)
    {
      _servo.writeMicroseconds(us);
    }

    void tick(void)
    {
      if(!_bActive)
        return;
        
      if(_pass > 0)
      {
        _pass--;
        return;
      }
      
      _pos += _step;
      _servo.writeMicroseconds(_pos);

      if(_pos == _target)
      {
        _bActive = false;
      }
      _pass = _delay;
    }
};


#define SERVOS 2
MotorMotion srv[SERVOS];

void setup()
{
  srv[0].Attach(9);
  srv[1].Attach(10);
  Serial.begin(9600);
}

enum
{
  mWaitCommand,
  mCollectNumber
};

enum
{
  cmdServoSelect, // s
  cmdGotoDegrees, // g
  cmdSetDelay,    // D
  cmdGotoDirect,  // d
  cmdNone
};

int mode = mWaitCommand;
int cmd = cmdNone;
String sBuffer("");
int servo = 0;

void TreatCommand(char ch)
{
  switch(ch)
  {
    case 's': // Servo Select
    case 'g': // Goto degrees
    case 'D': // Set Delay
    case 'd': // Goto Direct
      sBuffer = "";
      mode = mCollectNumber;
      break;
  }

  switch(ch)
  {
    case 's':
      cmd = cmdServoSelect;
      break;
    case 'g':
      cmd = cmdGotoDegrees;
      break;
    case 'D':
      cmd = cmdSetDelay;
      break;
    case 'd':
      cmd = cmdGotoDirect;
      break;
  }
}

void ApplyCommand(void)
{
  int number = sBuffer.toInt();
  switch(cmd)
  {
    case cmdServoSelect:
      if(number < SERVOS)
      {
        servo = number;
      }
      break;
    case cmdGotoDegrees:
      srv[servo].MotionTo(number);
      break;
    case cmdSetDelay:
      srv[servo].setDelay(number);
      break;
    case cmdGotoDirect:
      srv[servo].gotoDirect(number);
      break;
  }
}

void TreatCollectNumber(char ch)
{
  // Se for um digito, apendamos no buffer
  if(isdigit(ch))
  {
    sBuffer += (ch - '0');
  }
  else
  {
    // Não é digito, então acabou o número
    if(sBuffer.length() > 0)
    {
      // Existe conteúdo no buffer, então aplicamos o comando
      ApplyCommand();
    }
    // De qualquer maneira, passamos à esperar um comando novamente.
    mode = mWaitCommand;
  }
}

void loop()
{
  if(Serial.available())
  {
    char ch = Serial.read();
    switch(mode)
    {
      case mWaitCommand:
        TreatCommand(ch);
        break;
      case mCollectNumber:
        TreatCollectNumber(ch);
        break;
    }
  }
  for(int i = 0; i < SERVOS; i++)
  {
    srv[i].tick();
  }
}

