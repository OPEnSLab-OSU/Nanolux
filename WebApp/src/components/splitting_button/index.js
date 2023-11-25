import style from './style.css';
import {useEffect, useState} from "preact/hooks";
import Switch from 'preact-material-components/Switch';
import 'preact-material-components/Switch/style.css';
import {getIsStripSplitting, saveSplitting} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";
import {LabelSpinner} from "../spinner";

const SplittingButton = () => {

    const {isConnected} = useConnectivity();
    const [loading, setLoading] = useState(true);
    const [value, setValue] = useState(false);

    function nextState(){
        setValue(!value);
        saveSplitting(boolToInt(value));
    }

    function intToBool(b){
        if (b == 1){
            return true;
        }
        return false;
    }

    function boolToInt(b){
        if(b){
            return 1;
        }else{
            return 0;
        }
    }

    useEffect(() => {
        if (isConnected) {
            getIsStripSplitting().then(data => setValue(intToBool(data)));
            setLoading(false);
        }
        
    }, [isConnected])

    return (
        <div>
            {loading ? (
                <LabelSpinner />
            ) : (
                <div class='inline'>
                    <label>Enable virtual splitting  </label>
                    <button id='split_button' onClick={nextState}>
                        
                        {value.toString()}

                    </button>
                </div>
            )}
        </div>
    );
}

export default SplittingButton;
