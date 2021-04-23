package com.company;

import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import netscape.javascript.JSObject;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.io.*;
import java.util.*;

public class Main {
    public static void main(String[] args) {
	    JSONParser parser = new JSONParser();
        HashMap<String, ArrayList<String>> map = new HashMap<>();
        ArrayList<String> countryList = new ArrayList<>(Arrays.asList("VN", "CN", "AU"));
	    try {
	        Object obj = parser.parse(new FileReader("city.list.json"));
	        JSONArray list = (JSONArray) obj;
	        Iterator<JSONObject> iterator = list.iterator();
	        String currentCountry = "";
	        String currentCity = "";
	        while(iterator.hasNext()) {
                JSONObject currentObject = iterator.next();
                currentCountry = currentObject.get("country").toString();
                if(currentCountry.isEmpty() && currentCountry.isBlank()) {
                    continue;
                }
                if(countryList.contains(currentCountry)) {
                    currentCity = currentObject.get("name").toString();
                    if (!map.containsKey(currentCountry)) {
                        map.put(currentCountry, new ArrayList<>());
                    }
                    map.get(currentCountry).add(currentCity);
                }
            }
        } catch(IOException | ParseException e) {
	        System.out.println(e.getMessage());
        }
	    Iterator iterator = map.entrySet().iterator();
	    while(iterator.hasNext()) {
	        System.out.println(iterator.next());
        }
	    JSONObject jsonObject;
	    try {
            BufferedWriter writer = new BufferedWriter(new FileWriter("use.json"));
            Gson gson = new Gson();
            String json = gson.toJson(map);
            System.out.println(json);
            writer.write(json);
            writer.close();
        } catch(Exception e) {
            System.out.println(e.getMessage());
        }
    }
}
