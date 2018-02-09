#include "TURIPshell.h"

String TURIPshell(String line){
  size_t strBufLen = line.length();
  char* strBuf = new char[line.length() + 1];
  line.toCharArray(strBuf, line.length() + 1);
  String response = TURIPshell(strBuf);
  delete[] strBuf;
  return response;
}

String TURIPshell(const char* line){
  turipShellCommand cmd = turipShellCommandParser(line);
  turipShellResponse response;
  response.id = 0;
  if(cmd.depth == 1 && cmd.method == TURIP_METHOD_GET){
    response = turipShellLocalGet(&cmd);
  }else if(cmd.depth == 1 && cmd.method == TURIP_METHOD_POST){
    response = turipShellLocalPost(&cmd);
  }else if(cmd.depth == 2 && cmd.method == TURIP_METHOD_GET){
    response = turipShellBridgeGet(&cmd);
  }else if(cmd.depth == 2 && cmd.method == TURIP_METHOD_POST){
    response = turipShellBridgePost(&cmd);
  }else{
    response.statusCode = 400;
  }
  String strResponse;
  strResponse = "{\"status\":";
  if(cmd.depth == 0 && cmd.method == TURIP_METHOD_GET){
    strResponse += "200,\"protocol\":\"TURIP\",\"id\":\"";
    strResponse += turipIdIntToStr(TURIPserver.myId);
    strResponse += "\"";
    TURIPclient.scan();
    if(TURIPclient.numofDevices > 0){
      strResponse += ",\"bridge\":[";
      for(int i = 0;;){
        strResponse += "\"";
        strResponse += turipIdIntToStr(TURIPclient.devices[i].id);
        strResponse += "\"";
        i++;
        if(i == TURIPclient.numofDevices) break;
        strResponse += ",";
      }
      strResponse += "]";
    }
  }else{
    strResponse += response.statusCode;
    if(response.id != 0){
      strResponse += ",\"id\":\"";
      strResponse += turipIdIntToStr(response.id);
      strResponse += "\"";
    }
    if(response.statusCode == 200){
      strResponse += ",\"port\":";
      strResponse += response.port;
      strResponse += ",\"data\":";
      strResponse += response.data;
    }
  }
  strResponse += "}";
  return strResponse;
}

turipShellResponse turipShellLocalGet(turipShellCommand* cmd){
  turipShellResponse response;
  response.id = TURIPserver.myId;
  response.port = cmd->port;
  response.statusCode = 200;
  TURIPport* p = TURIPserver.getPort(cmd->port);
  if(p == NULL){
    response.statusCode = 404;
    return response;
  }
  uint8_t portData[64];
  p->transmit(portData, 64);

  switch(p->type){
    case TURIP_TYPE_UNKNOWN:
      response.statusCode = 500;
      break;
    case TURIP_TYPE_BOOL:
      if(portData[0] == 0){
        response.data = "false";
      } else if(portData[0] == 1) {
        response.data = "true";
      } else {
        response.statusCode = 500;
      }
      break;
    case TURIP_TYPE_UINT8:
      response.data = *((uint8_t*)portData);
      break;
    case TURIP_TYPE_INT8:
      response.data = *((int8_t*)portData);
      break;
    case TURIP_TYPE_UINT16:
      response.data = *((uint16_t*)portData);
      break;
    case TURIP_TYPE_INT16:
      response.data = *((int16_t*)portData);
      break;
    case TURIP_TYPE_UINT32:
      response.data = *((uint32_t*)portData);
      break;
    case TURIP_TYPE_INT32:
      response.data = *((int32_t*)portData);
      break;
    // case TURIP_TYPE_UINT64:
    //   response.data = *((uint64_t*)portData);
    //   break;
    // case TURIP_TYPE_INT64:
    //   response.data = *((int64_t*)portData);
    //   break;
    case TURIP_TYPE_FLOAT:
      response.data = *((float*)portData);
      break;
    case TURIP_TYPE_DOUBLE:
      response.data = *((double*)portData);
      break;
    case TURIP_TYPE_STRING:
      response.data = "\"";
      response.data += (char*)portData;
      response.data += "\"";
      break;
    default:
      response.statusCode = 500;
  }
  return response;
}

turipShellResponse turipShellLocalPost(turipShellCommand* cmd){
  turipShellResponse response;
  response.id = TURIPserver.myId;
  response.statusCode = 200;
  TURIPport* p = TURIPserver.getPort(cmd->port);
  if(p == NULL){
    response.statusCode = 404;
    return response;
  }
  response.port = cmd->port;
  union {
    uint8_t ui8;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    int8_t i8;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f;
    double d;
  } buf;

  switch(p->type){
    case TURIP_TYPE_UNKNOWN:
      response.statusCode = 500;
      break;
    case TURIP_TYPE_BOOL:
      if(!strcmp(cmd->data, "false")){
        buf.i8 = 0;
        response.data = "false";
      } else if(!strcmp(cmd->data, "true")){
        buf.i8 = 1;
        response.data = "true";
      } else {
        response.statusCode = 400;
        break;
      }
      p->receive(&buf.ui8);
      break;
    case TURIP_TYPE_UINT8:
      buf.ui8 = (uint8_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.ui8;
      break;
    case TURIP_TYPE_INT8:
      buf.i8 = (int8_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.i8;
      break;
    case TURIP_TYPE_UINT16:
      buf.ui16 = (uint16_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.ui16;
      break;
    case TURIP_TYPE_INT16:
      buf.i16 = (int16_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.i16;
      break;
    case TURIP_TYPE_UINT32:
      buf.ui32 = (uint32_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.ui32;
      break;
    case TURIP_TYPE_INT32:
      buf.i32 = (int32_t)atoi(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.i32;
      break;
    // case TURIP_TYPE_UINT64:
    //   buf.ui64 = (uint64_t)atoi(cmd->data);
    //   p->receive(&buf.ui8);
    //   response.data = buf.ui64;
    //   break;
    // case TURIP_TYPE_INT64:
    //   buf.i64 = (int64_t)atoi(cmd->data);
    //   p->receive(&buf.ui8);
    //   response.data = buf.i64;
    //   break;
    case TURIP_TYPE_FLOAT:
      buf.f = (float)atof(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.f;
      break;
    case TURIP_TYPE_DOUBLE:
      buf.d = (double)atof(cmd->data);
      p->receive(&buf.ui8);
      response.data = buf.d;
      break;
    case TURIP_TYPE_STRING:
      p->receive((uint8_t*)cmd->data);
      response.data = "\"";
      response.data += cmd->data;
      response.data += "\"";
      break;
    default:
      response.statusCode = 500;
  }
  return response;
}

turipShellResponse turipShellBridgeGet(turipShellCommand* cmd){
  turipShellResponse response;
  TURIPdevice dev;
  response.id = 0;

  if(dev.attach(cmd->id) == 0){
    response.statusCode = 200;
    response.id = cmd->id;
    response.port = cmd->port;

    switch(dev.getType(cmd->port)){
      case TURIP_TYPE_UINT8:
        response.data = dev.readUint8(cmd->port);
        break;
      case TURIP_TYPE_INT8:
        response.data = dev.readInt8(cmd->port);
        break;
      case TURIP_TYPE_UINT16:
        response.data = dev.readUint16(cmd->port);
        break;
      case TURIP_TYPE_INT16:
        response.data = dev.readInt16(cmd->port);
        break;
      case TURIP_TYPE_UINT32:
        response.data = dev.readUint32(cmd->port);
        break;
      case TURIP_TYPE_INT32:
        response.data = dev.readInt32(cmd->port);
        break;
      // case TURIP_TYPE_UINT64:
      //   response.data = dev.readUint64(cmd->port);
      //   break;
      // case TURIP_TYPE_INT64:
      //   response.data = dev.readInt64(cmd->port);
      //   break;
      case TURIP_TYPE_FLOAT:
        response.data = dev.readFloat(cmd->port);
        break;
      case TURIP_TYPE_DOUBLE:
        response.data = dev.readDouble(cmd->port);
        break;
      case TURIP_TYPE_STRING:
        response.data = "\"";
        response.data += dev.readString(cmd->port);
        response.data += "\"";
        break;
      default:
        response.statusCode = 500;
    }
  } else {
    response.statusCode = 404;
  }

  return response;
}

turipShellResponse turipShellBridgePost(turipShellCommand* cmd){
  turipShellResponse response;
  TURIPdevice dev;

  if(dev.attach(cmd->id) == 0){
    response.statusCode = 200;
    response.id = cmd->id;
    response.port = cmd->port;
    response.data = cmd->data;
    union {
      uint8_t ui8;
      uint16_t ui16;
      uint32_t ui32;
      uint64_t ui64;
      int8_t i8;
      int16_t i16;
      int32_t i32;
      int64_t i64;
      float f;
      double d;
    } buf;

    switch(dev.getType(cmd->port)){
      case TURIP_TYPE_UINT8:
        buf.ui8 = (uint8_t)atoi(cmd->data);
        dev.writeUint8(cmd->port, buf.ui8);
        response.data = buf.ui8;
        break;
      case TURIP_TYPE_INT8:
        buf.i8 = (int8_t)atoi(cmd->data);
        dev.writeInt8(cmd->port, buf.i8);
        response.data = buf.i8;
        break;
      case TURIP_TYPE_UINT16:
        buf.ui16 = (uint16_t)atoi(cmd->data);
        dev.writeUint16(cmd->port, buf.ui16);
        response.data = buf.ui16;
        break;
      case TURIP_TYPE_INT16:
        buf.i16 = (int16_t)atoi(cmd->data);
        dev.writeInt16(cmd->port, buf.i16);
        response.data = buf.i16;
        break;
      case TURIP_TYPE_UINT32:
        buf.ui32 = (uint32_t)atoi(cmd->data);
        dev.writeUint32(cmd->port, buf.ui32);
        response.data = buf.ui32;
        break;
      case TURIP_TYPE_INT32:
        buf.i32 = (int32_t)atoi(cmd->data);
        dev.writeInt32(cmd->port, buf.i32);
        response.data = buf.i32;
        break;
      // case TURIP_TYPE_UINT64:
      //   buf.ui64 = (uint64_t)atoi(cmd->data);
      //   dev.writeUint64(cmd->port, buf.ui64);
      //   response.data = buf.ui64;
      //   break;
      // case TURIP_TYPE_INT64:
      //   buf.i64 = (int64_t)atoi(cmd->data);
      //   dev.writeInt64(cmd->port, buf.i64);
      //   response.data = buf.i64;
      //   break;
      case TURIP_TYPE_FLOAT:
        buf.f = (float)atof(cmd->data);
        dev.writeFloat(cmd->port, buf.f);
        response.data = buf.f;
        break;
      case TURIP_TYPE_DOUBLE:
        buf.d = (double)atof(cmd->data);
        dev.writeDouble(cmd->port, buf.d);
        response.data = buf.d;
        break;
      case TURIP_TYPE_STRING:
        dev.writeString(cmd->port, cmd->data);
        response.data = "\"";
        response.data += cmd->data;
        response.data += "\"";
        break;
      default:
        response.statusCode = 500;
    }
  } else {
    response.statusCode = 404;
  }
  return response;
}

uint64_t turipIdStrToInt(const char* id){
  uint64_t _id = 0;
  char fullid[16 + 1];
  for (size_t i = 0; i < 16; i++) {
    fullid[i] = '0';
  }
  fullid[16] = '\0';
  String bufStr = id;
  int bufStlLength = bufStr.length();
  int separatorIndex = bufStr.indexOf('-');

  int index = 15;
  if(separatorIndex != -1){
    for (int i = bufStlLength - 1; i > separatorIndex; i--) {
      fullid[index--] = bufStr[i];
      if(index == 7) break;
    }
    index = 7;
    for (int i = separatorIndex - 1; i >= 0; i--) {
      fullid[index--] = bufStr[i];
      if(index < 0) break;
    }
  }else{
    for (int i = bufStlLength - 1; i >= 0; i--) {
      fullid[index--] = bufStr[i];
      if(index < 0) break;
    }
  }

  for (size_t i = 0; i < 16; i++) {
    uint8_t bin;
    switch (fullid[i]) {
      case '0': bin = 0x0; break;
      case '1': bin = 0x1; break;
      case '2': bin = 0x2; break;
      case '3': bin = 0x3; break;
      case '4': bin = 0x4; break;
      case '5': bin = 0x5; break;
      case '6': bin = 0x6; break;
      case '7': bin = 0x7; break;
      case '8': bin = 0x8; break;
      case '9': bin = 0x9; break;
      case 'a': bin = 0xa; break;
      case 'b': bin = 0xb; break;
      case 'c': bin = 0xc; break;
      case 'd': bin = 0xd; break;
      case 'e': bin = 0xe; break;
      case 'f': bin = 0xf; break;
    }
    _id |= (uint64_t)bin << (60 - 4 * i);
  }

  return _id;
}

String turipIdIntToStr(uint64_t id){
  uint32_t model = id >> 32;
  uint32_t serial = (uint32_t)id;
  char c[18];
  sprintf(c, "%x-%x", model, serial);
  String strid = c;
  return strid;
}

turipShellCommand turipShellCommandParser(const char* line){
  turipShellCommand parsed;
  int numofArgs = 0;
  char* args[5];
  int lineLength = strlen(line);
  char* raw = new char[lineLength + 1];
  strcpy(raw, line);

  for (size_t i = 0; i < lineLength; i++) {
    if(raw[i] == '\"'){
      raw[i] = '\0';
      args[numofArgs] = &raw[++i];
      numofArgs++;
      for(i++; i < lineLength; i++){
        if (raw[i] == '\"'){
          raw[i] = '\0';
          break;
        }
      }
    }else if(raw[i] == '\''){
      args[numofArgs] = &raw[i];
      numofArgs++;
      for(i++; i < lineLength; i++){
        if (raw[i] == '\'') break;
      }
    }else if(raw[i] != ' '){
      args[numofArgs] = &raw[i];
      numofArgs++;
      for(; i < lineLength; i++){
        if(raw[i] == ' ') break;
      }
    }
    if (raw[i] == ' '){
      raw[i] = '\0';
    }
  }

  parsed.method = TURIP_METHOD_UNKOWN;
  parsed.data = NULL;
  char* path = NULL;

  if(args[0][0] == '/'){
    path = args[0];
    if(numofArgs == 1){
      parsed.method = TURIP_METHOD_GET;
    }else if(numofArgs == 2){
      parsed.method = TURIP_METHOD_POST;
      parsed.data = args[1];
    }
  } else if(!strcmp(args[0], "get") || !strcmp(args[0], "GET")){
    if(args[1][0] == '/' && numofArgs == 2){
      parsed.method = TURIP_METHOD_GET;
      path = args[1];
    }
  } else if(!strcmp(args[0], "post") || !strcmp(args[0], "POST")){
    if(args[1][0] == '/' && numofArgs == 3){
      parsed.method = TURIP_METHOD_POST;
      parsed.data = args[2];
      path = args[1];
    }
  }

  if(path != NULL){
    int pathLength = strlen(path);
    parsed.depth = 0;
    char* pathSegments[5];
    for(int i = 0; i < pathLength; i++){
      if(path[i] != '/'){
        pathSegments[parsed.depth] = &path[i];
        parsed.depth++;
        for(; i < pathLength; i++){
          if(path[i] == '/') break;
        }
      }
      if(path[i] == '/') path[i] = '\0';
    }
    if(parsed.depth == 1){
      parsed.port = (uint8_t)atoi(pathSegments[0]);
    }else if(parsed.depth == 2){
      parsed.id = turipIdStrToInt(pathSegments[0]);
      parsed.port = (uint8_t)atoi(pathSegments[1]);
    }
  }

  return parsed;
}