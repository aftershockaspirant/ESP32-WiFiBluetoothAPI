

# ESP32-WiFiBluetoothAPI

This API allows devices to connect to it via Bluetooth, and respond to specific function requests.

## Table of Contents
- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)

## Introduction

The Marvel franchise is a vast and wildly popular universe of superheroes and stories that has captivated audiences since its inception. It all started in 1939 when Timely Comics, which would later become Marvel Comics, published its first comic book, "Marvel Comics #1." The Marvel franchise has achieved remarkable success, combining thrilling action, compelling storytelling, and memorable characters.

In this project, I set out to create a guide that would provide information about Marvel movies, such as details about the release date, box office details etc. To create this guide, I used an ESP32 module, which has WI-FI and Bluetooth Classic.


## Features

The module accesses an API (Application Program Interface) to obtain the necessary data, connects via Bluetooth to an Android smartphone that has the "ProiectIA" application installed and sends the obtained data to be displayed in an easily accessible list. The language used is C++, and the chosen development environment is the Arduino IDE.


## Installation

In order for the API to run on your machine, you first need an ESP32 module. the code has been written using the PlatformIO extension on VSCode.

Secondly, in order for the module to send and receive data, a Companion App is required, named "ProiectIA". it can be dowloaded at http://proiectia.bogdanflorea.ro/app

## Usage

Connect the esp32 module to your machine with an USB cable. The module activates by using the "Upload and Monitor" action provided by PlatformIO.

Open "ProiectIA" on your mobile device:
    For the bluetooth type select "Bluetooth Classic";
    For the project type select "API";
    For Team ID type "B19". This is required for the getNetworks request:

        The getNetworks request is the first request sent by the app and is used for obtaining the list of available WiFi networks available to the ESP32 device, as well as setting the teamId parameter, which will be a required parameter for all responses.

            {   
                action: 'getNetworks',
                teamId: string
            }
    



