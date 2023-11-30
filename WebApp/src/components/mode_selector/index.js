import {useEffect, useState} from "preact/hooks";
import 'preact-material-components/Switch/style.css';
import {getExclusiveMode, saveExclusiveMode} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import {LabelSpinner} from "../spinner";

// Radio buttons code reused from here:
// https://www.w3schools.com/tags/att_input_type_radio.asp
const ModeSelector = () => {

    const {isConnected} = useConnectivity();
    const [loading, setLoading] = useState(true);
    const [value, setValue] = useState(-1);

    useEffect(() => {
        if (isConnected) {
            getExclusiveMode().then(data => setValue(data.mode));
            setLoading(false);
        }
        
    }, [isConnected])

    function handleClick(){
        if(document.getElementById('dp').checked) {   
            saveExclusiveMode(1);
            setValue(1);
        }else if(document.getElementById('zl').checked){
            saveExclusiveMode(2);
            setValue(2);
        }else{
            saveExclusiveMode(0);
            setValue(0);
        }

    }

    return (
        <div>
            {loading ? (
                <LabelSpinner />
            ) : (
                <div>
                    Current selected mode:
                    <br></br>
                    <input type="radio" id="sp" name="mode" value="0" onChange={handleClick} checked={value==0}/>
                    <label for="sp">Single Pattern</label><br/>
                    <input type="radio" id="dp" name="mode" value="1" onChange={handleClick} checked={value==1}/>
                    <label for="dp">Dual Pattern</label><br/>
                    <input type="radio" id="zl" name="mode" value="2" onChange={handleClick} checked={value==2}/>
                    <label for="zl">Z-Layering</label> 
                </div>
            )}
        </div>
    );
}

export default ModeSelector;
