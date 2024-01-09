
import React from 'react';
import style from './style.css';
import NumericSlider from "../../components/numeric_slider";
import Save_Entry from '../../components/save_entry';
import {useState} from "preact/hooks";
import {useConnectivity} from "../../context/online_context";
import useInterval from "../../utils/use_interval";
import { saveData } from '../../utils/api';
import {saveInSlot} from "../../utils/api";
import {getPatternList} from "../../utils/api";
import DropDown from '../../components/drop_down';

const Controls = () => {

    // Controls to manage animation.
    var basicOpacity = 0;

    useInterval(() => {
        if(basicOpacity < 1){
            basicOpacity = basicOpacity + 0.1;
        }
        document.getElementById('anim').style = 'opacity:' + basicOpacity + ';';
    }, 5);

    

    const [currentTab, setCurrentTab] = useState("Basic Settings");
    const { isConnected } = useConnectivity();
	const [settings, setSettings] = useState({});

    // Global labels, mainly to standardize constants.
    const brightnessLabel = "Brightness";
    const smoothingLabel = "Smoothing";
    const primaryKey = "index1";
    const secondaryKey = "index2";
    const modeKey = "mode";
    const noiseKey = "Noise";
    const alphaKey = "Alpha";

    const Tab = ({name}) => {
        return (
            <div>
                { (name === currentTab) ? (
                    <button className={style.tab_active}>{name}</button>
                ) : (
                    <button className={style.tab_inactive} onClick={async function () {
                        setCurrentTab(name);
                        animFrame = 0;
                    }}>{name}</button>
                )}
            </div>
        );
    }

    // Updates a key/value settings pair and updates
    // the accompaning data field on the Audiolux.
    const handleChange = async (key, value) => {
        settings[key] = value;
        if(isConnected){
            saveData(key, value);
        }
    }

    // When a load is forced, we recieve the new pattern on the
    // web app, then update both the values on the web app and
    // the AudioLux at the same time:
    const loadSave = async (slot) => {
        getAll().then(data => setSettings(data[slot]));
        handleChange(brightnessLabel, settings[brightnessLabel]);
        handleChange(smoothingLabel, settings[smoothingLabel]);
        handleChange(primaryKey, settings[primaryKey]);
        handleChange(secondaryKey, settings[secondaryKey]);
        handleChange(modeKey, settings[modeKey]);
        handleChange(alphaKey, settings[alphaKey]);
        handleChange(noiseKey, settings[noiseKey]);
    }

    const [patterns, setPatterns] = useState([]);
    const [patternsFound, setPatternsFound] = useState(false);

    // The pattern list is updated every second.
    useInterval(() => {
        if (isConnected) {
            getPatternList().then(data => setPatterns(data));
            setPatternsFound(true);
        }
    }, 1000);

    const modes = [
        {index:0, name:"Single Pattern"},
        {index:1, name:"Dual Pattern"},
        {index:2, name:"Pattern Layering"}
    ];

    const modeOptions = modes.map(mode => {
        return <option key={mode.index} value={mode.index}>
            {mode.name}
        </option>
    });

    const patternOptions = patterns.map(pattern => {
        return <option key={pattern.index} value={pattern.index}>
            {pattern.name}
        </option>
    });

    return (
        <div>
            <table>
                <tr>
                    {/* To add more tab headers, add them here. */}
                    <td><Tab name="Basic Settings"></Tab></td>
                    <td><Tab name="All Settings"></Tab></td>
                    <td><Tab name="Saved Patterns"></Tab></td>
                </tr>
            </table>
            <br></br>

            { (currentTab === "Basic Settings" || currentTab == "All Settings") ? (
            <div id='anim'>
                <div className={style.settings_control}>
                    <DropDown
                        label="Primary Pattern"
                        api_key={primaryKey}
                        optionsList={patternOptions}
                        savedValue={settings[primaryKey]}
                        onValueChanged={handleChange}
                    />
                </div>
                <div className={style.settings_control}>
                    { settings && <NumericSlider
                        className={style.settings_control}
                        label={brightnessLabel}
                        savedValue={settings[brightnessLabel]}
                        min={0}
                        max={255}
                        onValueChanged={handleChange}
                    /> }
                </div>
                <div className={style.settings_control}>
                    { settings && <NumericSlider
                        className={style.settings_control}
                        label={smoothingLabel}
                        savedValue={settings[smoothingLabel]}
                        min={0}
                        max={255}
                        onValueChanged={handleChange}
                    /> }
                </div>
            </div>
            ) : (<div></div>)}

            { (currentTab === "All Settings") ? (
                <div id='anim'>
                    <div className={style.settings_control}>
                        <DropDown
                            label="Secondary Pattern"
                            api_key={secondaryKey}
                            optionsList={patternOptions}
                            savedValue={settings[secondaryKey]}
                            onValueChanged={handleChange}
                        />
                    </div>
                    <div className={style.settings_control}>
                        <DropDown
                            label="Current Mode"
                            api_key={modeKey}
                            optionsList={modeOptions}
                            savedValue={settings[modeKey]}
                            onValueChanged={handleChange}
                        />
					</div>
                    <div className={style.settings_control}>
                        { settings && <NumericSlider
                            className={style.settings_control}
                            label={alphaKey}
                            savedValue={settings[alphaKey]}
                            min={0}
                            max={255}
                            onValueChanged={handleChange}
                        /> }
                    </div>
                    <div className={style.settings_control}>
                        { settings && <NumericSlider
                            className={style.settings_control}
                            label={noiseKey}
                            savedValue={settings[noiseKey]}
                            min={0}
                            max={100}
                            onValueChanged={handleChange}
                        /> }
                    </div>
                </div>
            ) : (<div></div>)}

            { (currentTab === "Saved Patterns" ) ? (
            <div id='anim'>
                <div className={style.background0}>
                    <Save_Entry 
                        name="Default Pattern"
                        idx='0'
                    />
                </div>
                <br></br>
                <div className={style.background1}>
                    <Save_Entry 
                        name="Saved Pattern 1"
                        idx='1'
                        handleSave={saveInSlot}
                        handleLoad={loadSave}
                    />
                </div>
                <div className={style.background2}>
                    <Save_Entry 
                        name="Saved Pattern 2"
                        idx='2'
                        handleSave={saveInSlot}
                        handleLoad={loadSave}
                    />
                </div>
                <div className={style.background3}>
                    <Save_Entry 
                        name="Saved Pattern 3"
                        idx='3'
                        handleSave={saveInSlot}
                        handleLoad={loadSave}
                    />
                </div>
                <div className={style.background4}>
                    <Save_Entry 
                        name="Saved Pattern 4"
                        idx='4'
                        handleSave={saveInSlot}
                        handleLoad={loadSave}
                    />
                </div>
            </div>
            ) : (<div></div>)}

        </div>
        
    );
}

export default Controls;

