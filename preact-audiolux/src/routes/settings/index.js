
import { h, Component } from 'preact';
import style from './style.css';
import {Slider, MultiSlider} from 'preact-range-slider';
import sliderStyle from 'preact-range-slider/assets/index.css';
import { useState } from 'preact/hooks'


const Settings = () => {


  return (
    <div class={style.settings}>
      <h1>Settings</h1>
      <h4>Current Pattern</h4>
      <select name="patterns" id="patterns">
      <option value="blank">Blank</option>
      <option value="solid">Solid</option>
      <option value="trail">Trail</option>
      <option value="confetti">Confetti</option>
      <option value="vbar">VBar</option>
      </select>
      <h4>Noise Threshold</h4>
      <Slider style="sliderStyle" id="sliderNT"
      />
      <h4>Compression Threshold</h4>
      <Slider style="sliderStyle" id="sliderCT"/>
      <h4>Low Frequency Color</h4>
      <Slider style="sliderStyle" id="sliderLFC"/>
      <h4>High Frequency Color</h4>
      <Slider style="sliderStyle" id="sliderHFC"/>

    </div>
  );
};
export default Settings;
