import {useEffect, useState} from "preact/hooks";
import {loadFromSlot, saveInSlot} from "../../utils/api";
import {useConnectivity} from "../../context/online_context";

const Save_Entry = ({
    name,
    idx
}) => {
    const {isConnected} = useConnectivity();

    const handleLoad = async => {
        loadFromSlot(idx);
    }

    const handleSave = async => {
        saveInSlot(idx);
    }

    return (
        <div >
            <tr>
                <th width='150'>{name}</th>
                <th>
                    <button onClick={handleLoad}>Load Pattern</button>
                </th>
                <th>
                    <button onClick={handleSave}>Save Pattern</button>
                </th>
                
            </tr>
        </div>
    );
}

export default Save_Entry;
