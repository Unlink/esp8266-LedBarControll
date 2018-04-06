typedef struct {
  int red;
  int green;
  int blue;
  int trvanie;
  int prechod;
} AnimationStep;

struct {
  int red;
  int green;
  int blue;
} rgbLastColor;

AnimationStep* animacia = NULL;
int rgbAnimationMaxItems;
int rgbAnimationCurrentIndex;
int rgbAnimationCurrentDuration;

void rgbClearAnimation() {
  if (animacia != NULL) {
    delete[] animacia;
    animacia = NULL;
  }
  rgbAnimationMaxItems = 0;
}

void rgbCreateAnimationBuffer(int n) {
  rgbClearAnimation();
  animacia = new AnimationStep[n];
  rgbAnimationMaxItems = n;
  rgbAnimationCurrentIndex = 0;
  rgbAnimationCurrentDuration = 0;
}

void rgbAnimationSetValue(int i, int r, int g, int b, int trvanie, int prechod) {
  animacia[i].red = floor(r/(float)5)*5;
  animacia[i].green = floor(g/(float)5)*5;
  animacia[i].blue = floor(b/(float)5)*5;
  animacia[i].trvanie = trvanie;
  animacia[i].prechod = prechod;
  Serial.printf("Setting %d to %d:%d:%d for %d with %d\r\n", i, r, g, b, trvanie, prechod);
}

void rgbAnimate() {
  static unsigned long previousMillis = 0;
  if (animacia == NULL)
    return;
  unsigned long currentMillis = millis();
  long diff = currentMillis - previousMillis;
  if (diff < 40) {
    return;
  }
  previousMillis = currentMillis;

  //Ak sme presiahli celkový čas tejto farby
  while (rgbAnimationCurrentDuration + diff >= animacia[rgbAnimationCurrentIndex].trvanie + animacia[rgbAnimationCurrentIndex].prechod) {
    rgbAnimationCurrentIndex = (rgbAnimationCurrentIndex + 1) % rgbAnimationMaxItems;
    diff -= (animacia[rgbAnimationCurrentIndex].trvanie + animacia[rgbAnimationCurrentIndex].prechod) - rgbAnimationCurrentDuration;
    rgbAnimationCurrentDuration = 0;
  }
  rgbAnimationCurrentDuration += diff;
  if (rgbAnimationCurrentDuration <= animacia[rgbAnimationCurrentIndex].trvanie) {
    setRGB(animacia[rgbAnimationCurrentIndex].red, animacia[rgbAnimationCurrentIndex].green, animacia[rgbAnimationCurrentIndex].blue, true);
  }
  else {
    int nextI = (rgbAnimationCurrentIndex + 1) % rgbAnimationMaxItems;
    float progress = (rgbAnimationCurrentDuration - animacia[rgbAnimationCurrentIndex].trvanie) / (float)animacia[rgbAnimationCurrentIndex].prechod;
    int r = ((animacia[nextI].red - animacia[rgbAnimationCurrentIndex].red)*progress) + animacia[rgbAnimationCurrentIndex].red;
    int g = ((animacia[nextI].green - animacia[rgbAnimationCurrentIndex].green)*progress) + animacia[rgbAnimationCurrentIndex].green;
    int b = ((animacia[nextI].blue - animacia[rgbAnimationCurrentIndex].blue)*progress) + animacia[rgbAnimationCurrentIndex].blue;
    setRGB(r,g,b);
  }
  
}

void setRGB(int r, int g, int b) {
  setRGB(r, g, b, 1);
}

void setRGB(int r, int g, int b, int delta) {
  if (abs(rgbLastColor.red - r) >= delta) {
    rgbLastColor.red = r;
    analogWrite(RGB_PIN_R, r);
  }
  if (abs(rgbLastColor.green - g) >= delta) {
    rgbLastColor.green = g;
    analogWrite(RGB_PIN_G, g);
  }
  if (abs(rgbLastColor.blue - b) >= delta) {
    rgbLastColor.blue = b;
    analogWrite(RGB_PIN_B, b);
  }
}

