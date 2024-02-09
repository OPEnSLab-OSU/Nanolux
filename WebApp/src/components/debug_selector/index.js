import {useEffect, useState} from "preact/hooks";
//import 'preact-material-components/Switch/style.css';
import {getDebug, saveDebug} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import {LabelSpinner} from "../spinner";
import useInterval from "../../utils/use_interval";

// Radio buttons code reused from here:
// https://www.w3schools.com/tags/att_input_type_radio.asp
const DebugSelector = () => {

    const {isConnected} = useConnectivity();
    const [loading, setLoading] = useState(true);
    const [value, setValue] = useState(-1);

    useEffect(() => {
        if (isConnected) {
            getDebug().then(data => setValue(data.debug));
            setLoading(false);
        }
        
    }, [isConnected])

    function handleClick(){
        if(document.getElementById('debug').checked && value != 1) {   
            saveDebug(1);
            setValue(1);
        }else if(document.getElementById('sim').checked && value != 2){
            saveDebug(2);
            setValue(2);
        }else{
            saveDebug(0);
            setValue(0);
        }
        
    }

    useInterval(() => {
        if (isConnected) {
            refresh();
        }
    }, 1000);

    const refresh = () => {
        getDebug().then(data => setValue(data.debug));
    }

    return (
        <div>
            {loading ? (
                <LabelSpinner />
            ) : (
                <div>
                    <input type="checkbox" id="debug" name="debugmode" value="1" onChange={handleClick} checked={value==1}/>
                    <label for="sp">Enable Debug</label>
                    <input type="checkbox" id="sim" name="debugmode" value="2" onChange={handleClick} checked={value==2}/>
                    <label for="dp">Enable Simulator</label>
                </div>
            )}
        </div>
    );
}

export default DebugSelector;
