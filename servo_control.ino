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

    int getPos(void)
    {
      return _pos;
    }
    
    void MotionTo(int target)
    {
      // Se temos que ir onde estamos, pronto!
      if(target == _pos)
        return;
        
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

class ServoControl2d
{
private:
  MotorMotion _servos[2];
  int _nDelay;
  
public:
  ServoControl2d()
  {
    _nDelay = 10;
  }

  void Attach(int nPin1, int nPin2)
  {
    _servos[0].Attach(nPin1);
    _servos[1].Attach(nPin2);
  }

  void setDelay(int nDelay)
  {
    _nDelay = nDelay;
  }
  
  void tick(void)
  {
    _servos[0].tick();
    _servos[1].tick();
  }

  void MotionTo(int nTarget1, int nTarget2)
  {
    // Comandamos os servos:
    _servos[0].MotionTo(nTarget1);
    _servos[1].MotionTo(nTarget2);

    // Calculamos os delays:
    unsigned long delta1 = abs(nTarget1 - _servos[0].getPos());
    unsigned long delta2 = abs(nTarget2 - _servos[1].getPos());
    /*
      O delay total do maior caminho à ser percorrido deverá ser o mesmo delay do mais curto.
      delta1 * delay = delta2 * delay2
      
                delta1 * delay 
      delay2 = ----------------
                    delta2
    */
    Serial.print("Going from (");
    Serial.print(_servos[0].getPos());
    Serial.print(", ");
    Serial.print(_servos[1].getPos());
    Serial.print(") to (");
    Serial.print(nTarget1);
    Serial.print(", ");
    Serial.print(nTarget2);
    Serial.print("). Deltas: (");
    Serial.print(delta1);
    Serial.print(", ");
    Serial.print(delta2);
    Serial.print("). Delays: (");

    if(delta1 >= delta2)
    {
      unsigned long nGDelay = _nDelay;
      if(delta2 > 0)
        nGDelay = (delta1 * _nDelay)/ delta2;
        
      _servos[0].setDelay(_nDelay);
      _servos[1].setDelay(nGDelay);
      Serial.print(_nDelay);
      Serial.print(", ");
      Serial.print(nGDelay);
    }
    else
    {
      unsigned long nGDelay = _nDelay;
      if(delta1 > 0)
        nGDelay = (delta2 * _nDelay)/ delta1;
        
      _servos[0].setDelay(nGDelay);
      _servos[1].setDelay(_nDelay);
      Serial.print(nGDelay);
      Serial.print(", ");
      Serial.print(_nDelay);
    }

    Serial.println(").");
  }

  void gotoDirect(int n1, int n2)
  {
    _servos[0].gotoDirect(n1);
    _servos[1].gotoDirect(n2);
  }
};

ServoControl2d servos;

void setup()
{
  servos.Attach(9,10);
  Serial.begin(9600);
}

enum
{
  mWaitCommand,
  mCollectNumber,
  mCollectNumber1,
  mCollectNumber2
};

enum
{
  cmdGotoDegrees, // g
  cmdSetDelay,    // D
  cmdGotoDirect,  // d
  cmdNone
};

int mode = mWaitCommand;
int cmd = cmdNone;
String sBuffer1("");
String sBuffer2("");

void TreatCommand(char ch)
{
  switch(ch)
  {
    case 'g': // Goto degrees
    case 'd': // Goto Direct
      sBuffer1 = "";
      sBuffer2 = "";
      mode = mCollectNumber1;
      break;
    case 'D': // Set Delay
      sBuffer1 = "";
      mode = mCollectNumber;
      break;
  }

  switch(ch)
  {
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
  int number1 = sBuffer1.toInt();
  int number2 = sBuffer2.toInt();
  switch(cmd)
  {
    case cmdGotoDegrees:
      servos.MotionTo(number1, number2);
      break;
    case cmdSetDelay:
      servos.setDelay(number1);
      break;
    case cmdGotoDirect:
      servos.gotoDirect(number1, number2);
      break;
  }
}

void TreatCollectNumber(char ch)
{
  // Se for um digito, apendamos no buffer
  if(isdigit(ch))
  {
    switch(mode)
    {
      case mCollectNumber:
      case mCollectNumber1:
        sBuffer1 += (ch - '0');
        break;
      case mCollectNumber2:
        sBuffer2 += (ch - '0');
        break;
    }
  }
  else
  {
    if(mode == mCollectNumber1)
    {
      // Vamos ao próximo:
      mode = mCollectNumber2;
    }
    else
    {
      // Chegando aqui acabaram os dois:
      if( sBuffer1.length() > 0 && (mode == mCollectNumber || sBuffer2.length() > 0) )
      {
        // Existe conteúdo nos buffers, então aplicamos o comando
        ApplyCommand();
      }
      // De qualquer maneira, passamos à esperar um comando novamente.
      mode = mWaitCommand;
    }
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
      case mCollectNumber1:
      case mCollectNumber2:
        TreatCollectNumber(ch);
        break;
    }
  }
  servos.tick();
}

